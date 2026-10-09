#!/usr/bin/env python3
"""Serve a firmware build so the scale can update itself from this computer.

The scale looks for this server via mDNS (service "_coffeescale-fw._tcp")
when you long-press on the language screen of the firmware updater.

Usually started with:  pio run -t serve
Or directly:           python scripts/serve_firmware.py .pio/build/esp32dev/firmware.bin

The file is re-read on every request, so rebuilding while the server runs is
fine. If the scale already runs exactly this build, it gets "no update".
"""

import argparse
import hashlib
import http.server
import socket
import subprocess
import sys

SERVICE_TYPE = "_coffeescale-fw._tcp.local."
FIRMWARE_PATH = "/firmware.bin"


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


def make_handler(firmware_file):
    class FirmwareHandler(http.server.BaseHTTPRequestHandler):
        def do_GET(self):
            self.serve(send_body=True)

        def do_HEAD(self):
            self.serve(send_body=False)

        def serve(self, send_body):
            if self.path.split("?")[0] != FIRMWARE_PATH:
                self.send_error(404)
                return

            try:
                with open(firmware_file, "rb") as f:
                    data = f.read()
            except OSError as e:
                self.send_error(500, f"cannot read firmware: {e}")
                return

            md5 = hashlib.md5(data).hexdigest()
            running_md5 = self.headers.get("x-ESP32-sketch-md5", "")
            if running_md5 == md5:
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
                self.wfile.write(data)

        def log_message(self, format, *args):
            pass  # keep the output to the lines printed above

    return FirmwareHandler


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("firmware", help="path to firmware.bin")
    parser.add_argument("--port", type=int, default=0, help="HTTP port (default: any free port)")
    args = parser.parse_args()
    sys.stdout.reconfigure(line_buffering=True)  # show output when piped, e.g. by pio

    zeroconf = import_zeroconf()

    server = http.server.ThreadingHTTPServer(("0.0.0.0", args.port), make_handler(args.firmware))
    port = server.server_address[1]

    addresses = lan_ipv4_addresses()
    host = socket.gethostname().split(".")[0] or "laptop"
    info = zeroconf.ServiceInfo(
        SERVICE_TYPE,
        f"{host}.{SERVICE_TYPE}",
        addresses=[socket.inet_aton(a) for a in addresses],
        port=port,
        properties={"path": FIRMWARE_PATH},
        server=f"{host}.local.",
    )
    zc = zeroconf.Zeroconf()
    zc.register_service(info)

    print(f"Serving {args.firmware}")
    print(f"  at http://{addresses[0]}:{port}{FIRMWARE_PATH}" + (f" (also {', '.join(addresses[1:])})" if len(addresses) > 1 else ""))
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
