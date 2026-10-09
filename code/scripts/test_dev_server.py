"""Tests of dev_server.py, without mDNS.

Run with:  python3 -m unittest discover -s scripts -p 'test_*.py'
"""

import hashlib
import http.client
import http.server
import os
import re
import tempfile
import threading
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

    def test_other_paths_not_found(self):
        response, _ = self.server.request("POST", "/firmware.bin", RECORDING.encode())
        self.assertEqual(404, response.status)
        response, _ = self.server.request("GET", "/firmware.bin")
        self.assertEqual(404, response.status)


class SummarizeTest(unittest.TestCase):
    def test_summarize(self):
        self.assertEqual((2, 1.234, 1), dev_server.summarize(RECORDING))

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


if __name__ == "__main__":
    unittest.main()
