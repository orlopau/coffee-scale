"""Tests of dev_server.py, without mDNS.

Run with:  python3 -m unittest discover -s scripts -p 'test_*.py'
"""

import hashlib
import http.client
import http.server
import os
import re
import socket
import tempfile
import threading
import time
import unittest
from datetime import datetime
from unittest import mock

import dev_server

RECORDING = (
    "# coffee-scale recording\n"
    "# firmware: v1.9.0\n"
    "# grams_per_count: 0.002381\n"
    "ms,event,value\n"
    "0,sample,84012\n"
    "101,sample,84020\n"
    "1234,click,\n"
)


class Server:
    """A dev server on a free port, running in a thread."""

    def __init__(self, **handler_args):
        self.server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), dev_server.make_handler(**handler_args))
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()

    def request(self, method, path, body=None, headers=None):
        connection = http.client.HTTPConnection("127.0.0.1", self.server.server_address[1], timeout=5)
        connection.request(method, path, body=body, headers=headers or {})
        response = connection.getresponse()
        data = response.read()
        connection.close()
        return response, data

    def close(self):
        self.server.shutdown()
        self.server.server_close()


class RecordModeTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.record_dir = os.path.join(self.directory.name, "recordings")
        self.server = Server(record_dir=self.record_dir)

    def tearDown(self):
        self.server.close()
        self.directory.cleanup()

    def files(self):
        return sorted(os.listdir(self.record_dir)) if os.path.isdir(self.record_dir) else []

    def test_saves_recording(self):
        response, _ = self.server.request("POST", "/samples", RECORDING.encode())

        self.assertEqual(200, response.status)
        files = self.files()
        self.assertEqual(1, len(files))
        self.assertRegex(files[0], r"^\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}\.csv$")
        with open(os.path.join(self.record_dir, files[0]), "rb") as f:
            self.assertEqual(RECORDING.encode(), f.read())

    def test_same_second_gets_new_name(self):
        with mock.patch.object(dev_server, "timestamp", return_value=datetime(2026, 10, 9, 14, 3, 12)):
            self.server.request("POST", "/samples", RECORDING.encode())
            self.server.request("POST", "/samples", RECORDING.encode())
            self.server.request("POST", "/samples", RECORDING.encode())

        self.assertEqual(
            ["2026-10-09_14-03-12.csv", "2026-10-09_14-03-12_2.csv", "2026-10-09_14-03-12_3.csv"], self.files()
        )

    def test_rejects_non_recording(self):
        response, _ = self.server.request("POST", "/samples", b"hello")

        self.assertEqual(400, response.status)
        self.assertEqual([], self.files())

    def send_partial(self, close):
        """Announces the whole recording but sends only half of it, like an upload cut off by Wi-Fi."""
        data = RECORDING.encode()
        sock = socket.create_connection(("127.0.0.1", self.server.server.server_address[1]), timeout=5)
        sock.sendall(
            b"POST /samples HTTP/1.1\r\nHost: test\r\nContent-Length: %d\r\n\r\n" % len(data) + data[: len(data) // 2]
        )
        if close:
            sock.shutdown(socket.SHUT_WR)
        response = sock.recv(1024)
        sock.close()
        return response

    def test_rejects_cut_off_upload(self):
        response = self.send_partial(close=True)

        self.assertIn(b" 400 ", response.split(b"\r\n")[0])
        self.assertEqual([], self.files())

    def test_stalled_upload_times_out(self):
        self.server.close()
        with mock.patch.object(dev_server, "UPLOAD_TIMEOUT_S", 0.5):
            self.server = Server(record_dir=self.record_dir)
        started = time.monotonic()

        self.send_partial(close=False)

        self.assertLess(time.monotonic() - started, 3)
        self.assertEqual([], self.files())

    def test_other_paths_not_found(self):
        response, _ = self.server.request("POST", "/firmware.bin", RECORDING.encode())
        self.assertEqual(404, response.status)
        response, _ = self.server.request("GET", "/firmware.bin")
        self.assertEqual(404, response.status)


class SummarizeTest(unittest.TestCase):
    def test_summarize(self):
        self.assertEqual((2, 1.234, 1), dev_server.summarize(RECORDING))

    def test_summarize_skips_broken_rows(self):
        self.assertEqual((2, 1.234, 1), dev_server.summarize(RECORDING + "x,sample,1\n"))

    def test_summarize_empty(self):
        self.assertEqual((0, 0.0, 0), dev_server.summarize(RECORDING.split("ms,event,value\n")[0] + "ms,event,value\n"))


class ServeModeTest(unittest.TestCase):
    def test_serve_mode_unchanged(self):
        with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
            f.write(b"firmware bytes")
        server = Server(firmware_file=f.name)
        try:
            response, data = server.request("GET", "/firmware.bin")
            self.assertEqual(200, response.status)
            self.assertEqual(b"firmware bytes", data)
            self.assertEqual(hashlib.md5(b"firmware bytes").hexdigest(), response.getheader("x-MD5"))

            response, _ = server.request("POST", "/samples", RECORDING.encode())
            self.assertEqual(404, response.status)
        finally:
            server.close()
            os.remove(f.name)

    def test_slow_download_outlasts_timeout(self):
        # The scale writes the firmware to flash while downloading it, which takes
        # far longer than UPLOAD_TIMEOUT_S, but never stalls that long.
        firmware = os.urandom(50_000_000)
        with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
            f.write(firmware)
        with mock.patch.object(dev_server, "UPLOAD_TIMEOUT_S", 0.5):
            server = Server(firmware_file=f.name)
        try:
            sock = socket.socket()
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 16384)
            sock.settimeout(5)
            sock.connect(("127.0.0.1", server.server.server_address[1]))
            sock.sendall(b"GET /firmware.bin HTTP/1.1\r\nHost: scale\r\n\r\n")
            received = bytearray()
            started = time.monotonic()
            while True:
                chunk = sock.recv(1_000_000)
                if not chunk:
                    break
                received += chunk
                if time.monotonic() - started < 1.5:
                    time.sleep(0.01)
            sock.close()

            self.assertGreater(time.monotonic() - started, 1)
            self.assertTrue(received.endswith(firmware), f"got {len(received)} bytes")
        finally:
            server.close()
            os.remove(f.name)


class ReleaseModeTest(unittest.TestCase):
    """The dev server posing as a GitHub release, for automatic updates."""

    def setUp(self):
        with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
            f.write(b"firmware bytes")
        self.firmware = f.name
        self.server = Server(firmware_file=self.firmware, release="v9.9.9")

    def tearDown(self):
        self.server.close()
        os.remove(self.firmware)

    def test_latest_redirects_to_the_release(self):
        for method in ("HEAD", "GET"):
            response, data = self.server.request(method, "/releases/latest/download/firmware_de.bin")
            self.assertEqual(302, response.status, method)
            self.assertEqual("/releases/download/firmware-v9.9.9/firmware_de.bin", response.getheader("Location"))
            self.assertEqual(b"", data)

    def test_release_serves_the_firmware_like_github(self):
        # GitHub ignores the running firmware's MD5, so does the release
        md5 = hashlib.md5(b"firmware bytes").hexdigest()
        response, data = self.server.request(
            "GET", "/releases/download/firmware-v9.9.9/firmware_en.bin", headers={"x-ESP32-sketch-md5": md5}
        )
        self.assertEqual(200, response.status)
        self.assertEqual(b"firmware bytes", data)

    def test_other_versions_not_found(self):
        response, _ = self.server.request("GET", "/releases/download/firmware-v1.0.0/firmware_en.bin")
        self.assertEqual(404, response.status)

    def test_dev_updates_still_work(self):
        response, data = self.server.request("GET", "/firmware.bin")
        self.assertEqual(200, response.status)
        self.assertEqual(b"firmware bytes", data)


class NoReleaseTest(unittest.TestCase):
    def test_no_release_without_the_option(self):
        with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
            f.write(b"firmware bytes")
        server = Server(firmware_file=f.name)
        try:
            response, _ = server.request("GET", "/releases/latest/download/firmware_en.bin")
            self.assertEqual(404, response.status)
        finally:
            server.close()
            os.remove(f.name)


class FormatLogLinesTest(unittest.TestCase):
    NOW = datetime(2026, 11, 23, 12, 3, 44)

    def test_one_line(self):
        self.assertEqual(
            dev_server.format_log_lines(self.NOW, "192.168.1.37", b"I (5123) main: Tared!\n"),
            ["12:03:44 [192.168.1.37] I (5123) main: Tared!"])

    def test_several_lines_crlf_and_empty(self):
        self.assertEqual(
            dev_server.format_log_lines(self.NOW, "10.0.0.2", b"a\r\n\nb"),
            ["12:03:44 [10.0.0.2] a", "12:03:44 [10.0.0.2] b"])

    def test_broken_utf8_is_replaced(self):
        self.assertEqual(
            dev_server.format_log_lines(self.NOW, "10.0.0.2", b"x\xff\n"),
            ["12:03:44 [10.0.0.2] x\ufffd"])

    def test_control_characters_are_dropped_except_tab(self):
        # ESC and BEL go, so "[2J" stays as plain text; the tab stays
        self.assertEqual(
            dev_server.format_log_lines(self.NOW, "10.0.0.2", b"a\x1b[2Jb\x07\tc\n"),
            ["12:03:44 [10.0.0.2] a[2Jb\tc"])


class LogReceiverTest(unittest.TestCase):
    def test_prints_received_lines(self):
        lines = []
        receiver = dev_server.LogReceiver(host="127.0.0.1", output=lines.append)
        receiver.start()
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
                s.sendto(b"I (1) main: one\nI (2) main: two\n", ("127.0.0.1", receiver.port))
            deadline = time.time() + 2
            while len(lines) < 2 and time.time() < deadline:
                time.sleep(0.01)
        finally:
            receiver.close()
        self.assertEqual([l.split("] ", 1)[1] for l in lines], ["I (1) main: one", "I (2) main: two"])
        self.assertIn("[127.0.0.1]", lines[0])


class LogFileTest(unittest.TestCase):
    def test_prints_and_saves_every_line(self):
        printed = []
        with tempfile.TemporaryDirectory() as directory:
            log = dev_server.LogFile(os.path.join(directory, "logs"), output=printed.append)
            self.assertFalse(os.path.exists(log.path))
            log("12:00:00 [10.0.0.2] one")
            log("12:00:01 [10.0.0.2] two")
            log.close()
            with open(log.path, encoding="utf-8") as f:
                self.assertEqual(f.read(), "12:00:00 [10.0.0.2] one\n12:00:01 [10.0.0.2] two\n")
            self.assertTrue(os.path.basename(log.path).startswith("scale_"))
        self.assertEqual(printed[0], "12:00:00 [10.0.0.2] one")
        self.assertIn(log.path, printed[1])
        self.assertEqual(printed[2], "12:00:01 [10.0.0.2] two")

    def test_no_line_no_file(self):
        with tempfile.TemporaryDirectory() as directory:
            log = dev_server.LogFile(directory, output=lambda line: None)
            log.close()
            self.assertEqual(os.listdir(directory), [])


class ServicePropertiesTest(unittest.TestCase):
    def test_serve_with_log(self):
        self.assertEqual(dev_server.service_properties(False, 5000), {"path": "/firmware.bin", "log": "5000"})

    def test_record_with_log(self):
        self.assertEqual(dev_server.service_properties(True, 5000), {"record": "/samples", "log": "5000"})

    def test_no_log(self):
        self.assertEqual(dev_server.service_properties(False, None), {"path": "/firmware.bin"})


if __name__ == "__main__":
    unittest.main()
