#!/usr/bin/env python3
"""Development server for the scale, found by it via mDNS (service
"_coffeescale-fw._tcp") when you long-press on the language screen of the
firmware updater. It runs in one of two modes:

Serve a firmware build, so the scale can update itself from this computer:
    pio run -t serve
    python scripts/dev_server.py .pio/build/esp32dev/firmware.bin

The file is re-read on every request, so rebuilding while the server runs is
fine. If the scale already runs exactly this build, it gets "no update".

Pose as a GitHub release of a version, so the scale's automatic updates find
it (build the scale with RELEASES_URL, see the README):
    python scripts/dev_server.py .pio/build/esp32dev/firmware.bin --release v9.9.9 --port 8000

Receive recordings of the scale's load cell, see the README:
    pio run -t record
    python scripts/dev_server.py --record test/recordings

Each recording the scale uploads is saved as <date>_<time>.csv.
"""

import argparse
import hashlib
import http.server
import os
import socket
import subprocess
import sys
from datetime import datetime

SERVICE_TYPE = "_coffeescale-fw._tcp.local."
FIRMWARE_PATH = "/firmware.bin"
# like GitHub's: the latest release redirects to the release's own download
LATEST_RELEASE_PREFIX = "/releases/latest/download/"
RELEASE_PREFIX = "/releases/download/"
RECORD_PATH = "/samples"
RECORDING_FIRST_LINE = "# coffee-scale recording\n"
COLUMNS_LINE = "ms,event,value"
# an upload that stalls this long is given up, so it doesn't block a thread forever
UPLOAD_TIMEOUT_S = 10
# the scale reads well over this per second, see serve()
DOWNLOAD_PIECE_BYTES = 65536


def import_zeroconf():
    try:
        import zeroconf  # noqa: F401
    except ImportError:
        print("Installing the 'zeroconf' package for mDNS...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", "--quiet", "zeroconf"])
    import zeroconf

    return zeroconf


def lan_ipv4_addresses():
    """IPv4 address of this computer that the scale can reach.

    Only the address of the default route is announced. Announcing every
    interface also announces unreachable ones (e.g. Docker bridges), and the
    scale may pick one of those.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.connect(("192.0.2.1", 9))  # TEST-NET, nothing is sent
        return [s.getsockname()[0]]


def timestamp():
    return datetime.now()


def summarize(text):
    """Number of samples, duration in seconds and number of clicks of a recording."""
    samples = clicks = 0
    last_ms = 0
    in_data = False
    for line in text.splitlines():
        if not in_data:
            in_data = line == COLUMNS_LINE
            continue
        fields = line.split(",")
        if len(fields) != 3 or not fields[0].isdigit():
            continue
        if fields[1] == "sample":
            samples += 1
        elif fields[1] == "click":
            clicks += 1
        last_ms = int(fields[0])
    return samples, last_ms / 1000, clicks


def save_recording(record_dir, data):
    """Saves the recording under a name from the current time, never over an existing one."""
    os.makedirs(record_dir, exist_ok=True)
    stem = timestamp().strftime("%Y-%m-%d_%H-%M-%S")
    name = f"{stem}.csv"
    number = 1
    while True:
        try:
            # "x" fails if the file exists, so two uploads in the same second can't race
            with open(os.path.join(record_dir, name), "xb") as f:
                f.write(data)
            return name
        except FileExistsError:
            number += 1
            name = f"{stem}_{number}.csv"


def make_handler(firmware_file=None, record_dir=None, release=None):
    class DevServerHandler(http.server.BaseHTTPRequestHandler):
        timeout = UPLOAD_TIMEOUT_S

        def do_GET(self):
            self.serve(send_body=True)

        def do_HEAD(self):
            self.serve(send_body=False)

        def do_POST(self):
            if record_dir is None or self.path.split("?")[0] != RECORD_PATH:
                self.send_error(404)
                return

            length = int(self.headers.get("Content-Length", 0))
            try:
                data = self.rfile.read(length)
            except OSError:  # includes the timeout
                print(f"{self.client_address[0]} stopped sending, the recording was not saved")
                self.close_connection = True
                return
            if len(data) != length:
                # the scale lost the connection and will retry, a partial file would look complete
                print(f"{self.client_address[0]} sent {len(data)} of {length} bytes, the recording was not saved")
                self.send_error(400, "incomplete recording")
                return
            text = data.decode("utf-8", errors="replace")
            if not text.startswith(RECORDING_FIRST_LINE):
                print(f"{self.client_address[0]} sent something that is not a recording")
                self.send_error(400, "not a recording")
                return

            name = save_recording(record_dir, data)
            samples, seconds, clicks = summarize(text)
            print(f"Saved {name}: {samples} samples, {seconds:.1f} s, {clicks} clicks")
            self.send_response(200)
            self.send_header("Content-Length", "0")
            self.end_headers()

        def serve(self, send_body):
            path = self.path.split("?")[0]
            if firmware_file is not None and release is not None:
                if path.startswith(LATEST_RELEASE_PREFIX):
                    name = path[len(LATEST_RELEASE_PREFIX):]
                    print(f"{self.client_address[0]} asks for the latest release, it is {release}")
                    self.send_response(302)
                    self.send_header("Location", f"{RELEASE_PREFIX}firmware-{release}/{name}")
                    self.send_header("Content-Length", "0")
                    self.end_headers()
                    return
                if path.startswith(f"{RELEASE_PREFIX}firmware-{release}/"):
                    # GitHub has no "no update" answer
                    self.send_firmware(send_body, not_modified=False)
                    return
            if firmware_file is None or path != FIRMWARE_PATH:
                self.send_error(404)
                return
            self.send_firmware(send_body, not_modified=True)

        def send_firmware(self, send_body, not_modified):
            """The firmware, or 304 if not_modified and the scale runs exactly this build."""
            try:
                with open(firmware_file, "rb") as f:
                    data = f.read()
            except OSError as e:
                self.send_error(500, f"cannot read firmware: {e}")
                return

            md5 = hashlib.md5(data).hexdigest()
            running_md5 = self.headers.get("x-ESP32-sketch-md5", "")
            if not_modified and running_md5 == md5:
                print(f"{self.client_address[0]} already runs this build, no update")
                self.send_response(304)
                self.end_headers()
                return

            print(f"{self.client_address[0]} is downloading the firmware ({len(data)} bytes, md5 {md5})")
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(data)))
            # lets the scale verify the image before switching to it
            self.send_header("x-MD5", md5)
            self.end_headers()
            if send_body:
                # The timeout limits a whole write, and the scale downloads slower
                # than UPLOAD_TIMEOUT_S allows for all of it, as it writes flash meanwhile.
                # In pieces, only a scale that stops reading is given up.
                for start in range(0, len(data), DOWNLOAD_PIECE_BYTES):
                    self.wfile.write(data[start:start + DOWNLOAD_PIECE_BYTES])

        def log_message(self, format, *args):
            pass  # keep the output to the lines printed above

    return DevServerHandler


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("firmware", nargs="?", help="path to firmware.bin, to serve it")
    mode.add_argument("--record", metavar="DIR", help="receive recordings and save them in DIR")
    parser.add_argument("--port", type=int, default=0, help="HTTP port (default: any free port)")
    parser.add_argument(
        "--release",
        metavar="VERSION",
        help="also pose as the GitHub release VERSION, e.g. v9.9.9, for automatic updates (use with --port)",
    )
    args = parser.parse_args()
    if args.release and args.record:
        parser.error("--release serves a firmware, it doesn't go with --record")
    sys.stdout.reconfigure(line_buffering=True)  # show output when piped, e.g. by pio

    zeroconf = import_zeroconf()

    handler = make_handler(firmware_file=args.firmware, record_dir=args.record, release=args.release)
    server = http.server.ThreadingHTTPServer(("0.0.0.0", args.port), handler)
    port = server.server_address[1]

    addresses = lan_ipv4_addresses()
    host = socket.gethostname().split(".")[0] or "laptop"
    # the scale updates if the server offers "path", and records if it offers "record"
    properties = {"record": RECORD_PATH} if args.record else {"path": FIRMWARE_PATH}
    info = zeroconf.ServiceInfo(
        SERVICE_TYPE,
        f"{host}.{SERVICE_TYPE}",
        addresses=[socket.inet_aton(a) for a in addresses],
        port=port,
        properties=properties,
        server=f"{host}.local.",
    )
    zc = zeroconf.Zeroconf()
    try:
        zc.register_service(info)
    except zeroconf.NonUniqueNameException:
        zc.close()
        server.server_close()
        sys.exit("Another dev server runs on this computer (pio run -t serve or -t record), stop it first.")

    if args.record:
        print(f"Recording to {os.path.abspath(args.record)}")
        print(f"  at http://{addresses[0]}:{port}{RECORD_PATH}")
    else:
        print(f"Serving {args.firmware}")
        print(f"  at http://{addresses[0]}:{port}{FIRMWARE_PATH}")
        if args.release:
            print(f"  as release {args.release}: build the scale with RELEASES_URL=http://{addresses[0]}:{port}/releases")
    print("On the scale: hold the button while switching it on, then long-press on the language screen.")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        zc.unregister_service(info)
        zc.close()
        server.server_close()


if __name__ == "__main__":
    main()
