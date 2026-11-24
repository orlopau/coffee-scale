#!/usr/bin/env python3
"""Play a Mahlkönig Xenia on this computer for an E64 WS grinder, and log
everything the grinder sends. It answers like the scale (src/grinder/xenia.cpp),
so a shot works the same, but nothing is hidden: every connection, every
request with its headers and body, every reply, requests nobody knows, bytes
that aren't HTTP, and the grinder's mDNS questions while it scans.

    sudo python3 scripts/fake_xenia.py

Port 80 needs root (the grinder's Edit Host takes no port). Then:

1. Turn the scale's "Mahlkönig grinder" setting off, so the grinder can't pick
   the scale.
2. On the grinder: Connectivity -> Machine To Machine -> Enable Xenia, then
   Configuration -> Edit Host: the IP printed at the start. (For the Scan
   instead, add --announce: it answers as xenia-coffee.local and xenia.local,
   which clashes with the scale if its setting is on.)
3. Grind a GbS recipe and press the knob. The brew ratio POST and the knob
   press POST show up here, each three times about 8.5 s apart: the grinder
   repeats them, and like the scale this takes a knob press within 20 s of the
   last one as a repeat. With no knob press, the log shows what the grinder
   did send, and what it was told before.

The grinder polls about every 0.8 s on one connection. Polls that only repeat
the last one are summed up every 10 s; "v" shows each in full.

While it runs, type a command and Enter:
    (Enter)  end the running shot, at the time so far
    a        abort the running shot (MA_EXTRACTION_STATUS 2)
    k        a knob press from here, as if the grinder sent scripts/execute
    s N      always answer MA_STATUS N (0 off, 1 on, 2 eco, 3 serving, 4 finishing, 5, 6); "s" alone stops it
    t N      answer BG_SENS_TEMP N; "t" alone goes back to 93.0
    w N      answer TANK_LEVEL N; "w" alone goes back to 1
    v        every poll in full, or back to one line per poll
    q        quit

Everything also goes to a JSON-lines file (--log), one object per event.
"""

import argparse
import datetime
import json
import os
import re
import select
import shutil
import socket
import struct
import subprocess
import sys
import threading
import time

MDNS_GROUP = "224.0.0.251"
MDNS_PORT = 5353
# what the scale (and gbs-anywhere) accept as a grinder's body
MAX_BODY = 4096
MAX_HEAD = 16384
# the scale holds Finishing this long, and until a poll has seen it
FINISHING_MS = 6000
# no shot after the knob press: aborted after this long, like the scale
NO_SHOT_MS = 180_000
# the E64 WS sends each knob press three times, 8.2-8.8 s apart: like the scale, a knob press
# this soon after the one that started the last shot is a repeat of it
START_REPEAT_MS = 20_000
# the grinder polls every 0.4-1.9 s; polls that only repeat themselves are summed up this often,
# a longer gap is shown at once
POLL_SUMMARY_MS = 10_000
POLL_GAP_MS = 2_500
STATUS_NAMES = {0: "off", 1: "on", 2: "eco", 3: "serving", 4: "finishing", 5: "testing", 6: "fault"}


def now_ms():
    return int(time.monotonic() * 1000)


def stamp():
    return datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]


class Log:
    """Prints to the terminal and writes every event as a JSON line."""

    def __init__(self, path):
        self.lock = threading.Lock()
        self.file = open(path, "a", encoding="utf-8") if path else None
        self.verbose_polls = False

    def say(self, tag, text):
        with self.lock:
            print(f"{stamp()} {tag:7} {text}", flush=True)

    def event(self, kind, **fields):
        if not self.file:
            return
        fields = {"t": datetime.datetime.now().isoformat(timespec="milliseconds"), "kind": kind, **fields}
        with self.lock:
            self.file.write(json.dumps(fields, ensure_ascii=False) + "\n")
            self.file.flush()


class Xenia:
    """The machine the grinder sees: Ready (1), Running (3), Finishing (4), as the scale plays it."""

    def __init__(self, log, serial, shot_seconds):
        self.log = log
        self.serial = serial
        self.shot_ms = int(shot_seconds * 1000) if shot_seconds > 0 else None
        self.lock = threading.Lock()
        self.phase = "Ready"
        # the knob press that started the last shot, None before the first
        self.started = None
        self.finishing_since = 0
        self.seen = False
        self.aborted = False
        self.time_ms = 0
        self.weight = 0.0
        self.brew_weight = None
        # overrides from the keyboard
        self.force_status = None
        self.temp = 93.0
        self.tank = 1

    def _move(self, phase, why):
        self.log.say("XENIA", f"{self.phase} -> {phase}: {why}")
        self.log.event("phase", old=self.phase, new=phase, why=why)
        self.phase = phase
        self.seen = False

    def _tick(self, now):
        if self.phase == "Running":
            elapsed = now - self.started
            if self.shot_ms is not None and elapsed >= self.shot_ms:
                self._end(now, aborted=False, time_ms=self.shot_ms, why=f"the shot's {self.shot_ms} ms are up")
            elif elapsed >= NO_SHOT_MS:
                self._end(now, aborted=True, time_ms=elapsed, why="no end within 3 minutes")
        elif self.phase == "Finishing" and self.seen and now - self.finishing_since >= FINISHING_MS:
            self._move("Ready", "Finishing was seen and held 6 s")

    def _end(self, now, aborted, time_ms, why):
        self.aborted = aborted
        self.time_ms = time_ms
        self.weight = 0.0 if aborted else (self.brew_weight or 40.0)
        self.finishing_since = now
        self._move("Finishing", f"{'aborted' if aborted else 'done'}, {time_ms} ms, {self.weight:.1f} g ({why})")

    def knob_press(self, source):
        with self.lock:
            now = now_ms()
            self._tick(now)
            if self.started is not None and now - self.started < START_REPEAT_MS:
                self.log.say("XENIA", f"knob press ({source}) {now - self.started} ms after the last one: a repeat, ignored ({self.phase})")
                return
            if self.phase == "Running":
                self.log.say("XENIA", f"knob press ({source}) while Running: the shot runs on")
                return
            self.started = now
            self.time_ms = 0
            self.weight = 0.0
            self.aborted = False
            target = f"{self.brew_weight:.1f} g" if self.brew_weight else "none, the grinder sent no brew ratio"
            self._move("Running", f"knob press ({source}), target {target}")

    def end(self, aborted):
        with self.lock:
            now = now_ms()
            self._tick(now)
            if self.phase != "Running":
                self.log.say("XENIA", f"no shot is running ({self.phase})")
                return
            self._end(now, aborted, now - self.started, "from the keyboard")

    def brew_ratio(self, body):
        try:
            value = json.loads(body).get("SYNC_BEVERAGE_WEIGHT")
            grams = float(value)
        except (ValueError, TypeError, AttributeError):
            return "no readable SYNC_BEVERAGE_WEIGHT, Brew Weight unchanged"
        if not grams > 0:
            return f"SYNC_BEVERAGE_WEIGHT {value!r}, Brew Weight unchanged"
        with self.lock:
            was = self.brew_weight
            self.brew_weight = grams
        return f"Brew Weight {grams:.1f} g (was {'none' if was is None else f'{was:.1f} g'})"

    def mako(self):
        with self.lock:
            now = now_ms()
            self._tick(now)
            status, extraction, time_ms, weight = 1, 0, self.time_ms, self.weight
            if self.phase == "Running":
                status, extraction, time_ms, weight = 3, 1, now - self.started, 0.0
            elif self.phase == "Finishing":
                status, extraction = 4, 2 if self.aborted else 1
            if not self.seen and self.phase != "Ready":
                self.log.say("XENIA", f"the grinder sees {self.phase}")
            self.seen = True
            if self.force_status is not None:
                status = self.force_status
            return (
                f'{{"MA_STATUS":{status},"MA_EXTRACTION_STATUS":{extraction},"REAL_EXTRACTION_TIME_MS":{time_ms},'
                f'"PU_SENS_FLOW_METER_VOLUME":{weight:.1f},"BG_SET_TEMP":93.0,"BG_SENS_TEMP":{self.temp:.1f},'
                f'"TANK_LEVEL":{self.tank},"script":"","TIMESTAMP":""}}'
            )

    def identity(self):
        return (
            f'{{"MA_TYPE":1,"MA_SUBTYPE":0,"MA_SN":"{self.serial}","FW_VERSION_MAJOR":4,'
            '"FW_VERSION_MINOR":226,"ESP_FW_MAJOR":3,"ESP_FW_MINOR":87,'
            '"MA_MAX_AMPERE":13,"MA_FIX_WATER_SUPPLY":0,"BLE":0,"MA_DELAY":-1}'
        )


def printable(data, limit=600):
    """Bytes as text where they are text, with escapes where they aren't."""
    text = data[:limit].decode("utf-8", errors="backslashreplace")
    text = "".join(c if c.isprintable() or c in "\n" else repr(c)[1:-1] for c in text)
    return text + (f"... ({len(data)} B)" if len(data) > limit else "")


def hexdump(data, limit=256):
    lines = []
    for i in range(0, min(len(data), limit), 16):
        chunk = data[i : i + 16]
        ascii_ = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        lines.append(f"{i:04x}  {chunk.hex(' '):47}  {ascii_}")
    return "\n".join(lines)


class Connection:
    """One TCP connection from the grinder (or anyone): reads HTTP requests and answers them."""

    METHODS = (b"GET ", b"POST ", b"PUT ", b"DELETE ", b"HEAD ", b"OPTIONS ", b"PATCH ")

    def __init__(self, server, sock, peer):
        self.server = server
        self.sock = sock
        self.peer = f"{peer[0]}:{peer[1]}"
        self.buffer = b""

    def _recv(self):
        data = self.sock.recv(4096)
        if not data:
            raise ConnectionError("closed by the client")
        self.buffer += data

    def _read_head(self):
        while b"\r\n\r\n" not in self.buffer:
            # stray line ends between requests are allowed
            self.buffer = self.buffer.lstrip(b"\r\n")
            if len(self.buffer) > MAX_HEAD:
                raise ValueError("a head over 16 KiB")
            if self.buffer and not any(m.startswith(self.buffer[:8]) or self.buffer.startswith(m) for m in self.METHODS):
                raise ValueError("not HTTP")
            self._recv()
        head, self.buffer = self.buffer.split(b"\r\n\r\n", 1)
        return head

    def _read_body(self, headers):
        if "chunked" in headers.get("transfer-encoding", "").lower():
            body = b""
            while True:
                while b"\r\n" not in self.buffer:
                    self._recv()
                size_line, self.buffer = self.buffer.split(b"\r\n", 1)
                size = int(size_line.split(b";")[0], 16)
                while len(self.buffer) < size + 2:
                    self._recv()
                body += self.buffer[:size]
                self.buffer = self.buffer[size + 2 :]
                if size == 0:
                    return body
        length = int(headers.get("content-length", "0") or 0)
        while len(self.buffer) < length:
            self._recv()
        body, self.buffer = self.buffer[:length], self.buffer[length:]
        return body

    def run(self):
        log = self.server.log
        log.say("TCP", f"{self.peer} connected to port {self.server.port}")
        log.event("connect", peer=self.peer, port=self.server.port)
        self.sock.settimeout(self.server.idle_timeout)
        requests = 0
        why = "closed"
        try:
            while True:
                head = self._read_head()
                requests += 1
                if not self.handle(head):
                    why = "closed by us"
                    break
        except socket.timeout:
            why = f"idle {self.server.idle_timeout:.0f} s, closed by us"
        except ValueError as error:
            why = str(error)
            log.say("TCP", f"{self.peer} sent something that isn't an HTTP request ({error}):\n{hexdump(self.buffer)}")
            log.event("garbage", peer=self.peer, error=str(error), data=self.buffer.hex())
        except Exception as error:  # noqa: BLE001 - a bug here must not hide the request
            why = f"{type(error).__name__}: {error}"
            if not isinstance(error, (ConnectionError, OSError)):
                log.say("TCP", f"{self.peer}: error while answering: {why}")
            if self.buffer:
                log.say("TCP", f"{self.peer} left {len(self.buffer)} B unanswered:\n{hexdump(self.buffer)}")
                log.event("partial", peer=self.peer, data=self.buffer.hex())
        finally:
            self.sock.close()
        log.say("TCP", f"{self.peer} gone after {requests} request(s): {why}")
        log.event("disconnect", peer=self.peer, requests=requests, why=why)

    def handle(self, head):
        log = self.server.log
        xenia = self.server.xenia
        lines = head.decode("latin-1").split("\r\n")
        request_line = lines[0]
        parts = request_line.split(" ")
        method, target = (parts[0], parts[1]) if len(parts) >= 2 else (request_line, "")
        version = parts[2] if len(parts) >= 3 else "HTTP/1.0"
        headers = {}
        for line in lines[1:]:
            name, _, value = line.partition(":")
            headers[name.strip().lower()] = value.strip()
        body = self._read_body(headers) if method.upper() != "GET" or "content-length" in headers else b""
        path = target.split("?", 1)[0].rstrip("/")
        get, post = method.upper() == "GET", method.upper() == "POST"

        status_line, note, kind = "200 OK", "", "unknown"
        if len(body) > MAX_BODY:
            status_line, reply, kind = "413 Payload Too Large", "", "too large"
        elif get and path == "/api/v2/mako":
            kind, reply = "poll", xenia.mako()
        elif get and path == "/api/v2/machine":
            kind, reply = "identity", xenia.identity()
        elif post and path == "/api/v2/brewratio":
            kind, reply = "brew ratio", "{}"
            note = xenia.brew_ratio(body)
        elif post and path == "/api/v2/scripts/execute":
            kind, reply = "knob press", "{}"
            xenia.knob_press(f"scripts/execute {printable(body, 80)}")
        else:
            reply = "{}"

        if kind in ("poll", "identity", "brew ratio", "knob press"):
            self.server.grinders.add(self.peer.rsplit(":", 1)[0])
        now = now_ms()
        gap, gap_ms = "", 0
        if kind == "poll":
            self.server.polls += 1
            if self.server.last_poll:
                gap_ms = now - self.server.last_poll
                gap = f", {gap_ms} ms after the last"
            self.server.last_poll = now
        log.event(
            "request",
            peer=self.peer,
            request=kind,
            request_line=request_line,
            headers=lines[1:],
            body=body.decode("utf-8", errors="backslashreplace"),
            reply_status=status_line,
            reply=reply,
            note=note,
        )

        # A poll is one line unless something about it is new; everything else in full.
        # the time runs while a shot runs, it alone makes no poll new
        signature = (kind, tuple(lines[1:]), re.sub(r'"REAL_EXTRACTION_TIME_MS":\d+', "", reply))
        brief = kind == "poll" and not log.verbose_polls and signature in self.server.seen_polls
        if kind == "poll":
            self.server.seen_polls.add(signature)
        tag = "UNKNOWN" if kind == "unknown" else "HTTP"
        summary = self.server.summary
        if brief:
            summary.append(gap_ms)
            if gap_ms > POLL_GAP_MS:
                log.say("HTTP", f"<- poll #{self.server.polls} from {self.peer}{gap}: a long gap -> MA_STATUS {mako_status(reply)}")
            if now - self.server.summary_since >= POLL_SUMMARY_MS:
                gaps = [g for g in summary if g]
                spread = f", {min(gaps)}-{max(gaps)} ms apart" if gaps else ""
                log.say("HTTP", f"<- {len(summary)} polls up to #{self.server.polls}{spread} -> MA_STATUS {mako_status(reply)}")
                summary.clear()
                self.server.summary_since = now
        else:
            if summary:
                log.say("HTTP", f"<- {len(summary)} polls like the last, up to #{self.server.polls - 1}")
                summary.clear()
            self.server.summary_since = now
            text = [f"<- {kind}: {request_line} from {self.peer}" + (f" (poll #{self.server.polls}{gap})" if gap else "")]
            text += [f"     {line}" for line in lines[1:]]
            if body:
                text.append(f"     body ({len(body)} B): {printable(body)}")
            if note:
                text.append(f"     {note}")
            text.append(f"-> {status_line} {reply}")
            log.say(tag, "\n".join(text))

        keep_alive = (
            not self.server.close
            and headers.get("connection", "").lower() != "close"
            and (version.upper() == "HTTP/1.1" or headers.get("connection", "").lower() == "keep-alive")
        )
        data = reply.encode()
        response = (
            f"{version if version.startswith('HTTP/') else 'HTTP/1.1'} {status_line}\r\n"
            f"Content-Type: application/json\r\nContent-Length: {len(data)}\r\n"
            f"Connection: {'keep-alive' if keep_alive else 'close'}\r\n\r\n"
        ).encode() + data
        self.sock.sendall(response)
        return keep_alive


def mako_status(reply):
    match = re.search(r'"MA_STATUS":(\d+)', reply)
    code = int(match.group(1)) if match else -1
    rest = re.search(r'"MA_EXTRACTION_STATUS":(\d+),"REAL_EXTRACTION_TIME_MS":(\d+)', reply)
    extra = f", extraction {rest.group(1)}, {rest.group(2)} ms" if rest and code != 1 else ""
    return f"{code} ({STATUS_NAMES.get(code, '?')}){extra}"


class HttpServer:
    def __init__(self, log, xenia, port, close, idle_timeout, grinders):
        self.log = log
        # the IPs that asked what only the grinder asks, for the mDNS lines
        self.grinders = grinders
        self.xenia = xenia
        self.port = port
        self.close = close
        self.idle_timeout = idle_timeout
        self.polls = 0
        self.last_poll = 0
        self.seen_polls = set()
        # the gaps of the polls not shown since the last line about them
        self.summary = []
        self.summary_since = 0
        self.sock = listen_tcp(port)

    def serve(self):
        while True:
            client, peer = self.sock.accept()
            threading.Thread(target=Connection(self, client, peer).run, daemon=True).start()


def listen_tcp(port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("0.0.0.0", port))
    sock.listen(16)
    return sock


def watch_port(log, sock, port):
    """Any other port the grinder might try: log the connection and what it sends first."""

    def one(client, peer):
        client.settimeout(5)
        try:
            data = client.recv(2048)
        except OSError:
            data = b""
        client.close()
        log.say("TCP", f"{peer[0]}:{peer[1]} connected to port {port}, sent {len(data)} B" + (f":\n{hexdump(data)}" if data else ""))
        log.event("other port", peer=f"{peer[0]}:{peer[1]}", port=port, data=data.hex())

    while True:
        client, peer = sock.accept()
        threading.Thread(target=one, args=(client, peer), daemon=True).start()


# --- mDNS: what the grinder asks while it scans ---

TYPES = {1: "A", 12: "PTR", 16: "TXT", 28: "AAAA", 33: "SRV", 47: "NSEC", 255: "ANY"}


def dns_name(data, offset):
    labels, jumped, end = [], False, offset
    for _ in range(64):
        length = data[offset]
        if length & 0xC0 == 0xC0:
            pointer = struct.unpack_from("!H", data, offset)[0] & 0x3FFF
            if not jumped:
                end = offset + 2
            jumped, offset = True, pointer
            continue
        offset += 1
        if length == 0:
            break
        labels.append(data[offset : offset + length].decode("utf-8", errors="backslashreplace"))
        offset += length
    return ".".join(labels) + ".", (end if jumped else offset)


def parse_mdns(data):
    """(is_response, questions [(name, type, unicast)], records [(name, type, value)])."""
    _, flags, qd, an, ns, ar = struct.unpack_from("!6H", data, 0)
    offset, questions, records = 12, [], []
    for _ in range(qd):
        name, offset = dns_name(data, offset)
        qtype, qclass = struct.unpack_from("!HH", data, offset)
        offset += 4
        questions.append((name, TYPES.get(qtype, str(qtype)), bool(qclass & 0x8000)))
    for _ in range(an + ns + ar):
        name, offset = dns_name(data, offset)
        rtype, _, _, length = struct.unpack_from("!HHIH", data, offset)
        offset += 10
        rdata = data[offset : offset + length]
        if rtype == 1 and length == 4:
            value = socket.inet_ntoa(rdata)
        elif rtype == 12:
            value = dns_name(data, offset)[0]
        elif rtype == 33:
            value = f"{dns_name(data, offset + 6)[0]}:{struct.unpack_from('!H', data, offset + 4)[0]}"
        elif rtype == 16:
            value = printable(rdata, 120)
        else:
            value = f"{length} B"
        records.append((name, TYPES.get(rtype, str(rtype)), value))
        offset += length
    return bool(flags & 0x8000), questions, records


def open_mdns(interface_ip):
    """Port 5353 next to avahi and the others there: multicast reaches every socket on it."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    if hasattr(socket, "SO_REUSEPORT"):
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    sock.bind(("", MDNS_PORT))
    sock.setsockopt(
        socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, socket.inet_aton(MDNS_GROUP) + socket.inet_aton(interface_ip)
    )
    return sock


def sniff_mdns(log, sock, interface_ip, everything, grinders):
    # a home network is chatty: only the grinder and the Xenia's names, unless --mdns-all
    interesting = re.compile(r"xenia|mako|mahl", re.I)
    recent = {}
    log.say("MDNS", f"listening on {MDNS_GROUP}:{MDNS_PORT} via {interface_ip}, showing the grinder and Xenia names")
    while True:
        data, (source, _) = sock.recvfrom(9000)
        if source == interface_ip:
            continue
        try:
            response, questions, records = parse_mdns(data)
        except (struct.error, IndexError):
            continue
        from_grinder = source in grinders
        if response:
            shown = [r for r in records if everything or from_grinder or interesting.search(r[0] + str(r[2]))]
            if not shown:
                continue
            text = ", ".join(f"{n} {t} {v}" for n, t, v in shown[:12])
            key = (source, "answer", text)
        else:
            if not (everything or from_grinder or interesting.search(str(questions))):
                continue
            text = ", ".join(f"{n} {t}{' (QU)' if u else ''}" for n, t, u in questions)
            key = (source, "query", text)
        # the same question again within 10 s is one line
        now = time.monotonic()
        if not everything and now - recent.get(key, -99) < 10:
            continue
        recent[key] = now
        mark = " <- THE GRINDER" if from_grinder else ""
        log.say("MDNS", f"{source} {'answers' if response else 'asks'}: {text}{mark}")
        log.event("mdns", source=source, response=response, questions=questions, records=records)


def announce(log, ip, names):
    """Answer for the Xenia's names over mDNS, with avahi."""
    if not shutil.which("avahi-publish"):
        log.say("MDNS", "avahi-publish not found, nothing announced (Edit Host with the IP works without it)")
        return []
    processes = []
    for name in names:
        processes.append(subprocess.Popen(["avahi-publish", "-a", "-R", f"{name}.local", ip], stderr=subprocess.PIPE))
    processes.append(
        subprocess.Popen(
            ["avahi-publish", "-s", names[0], "_http._tcp", "80"], stdout=subprocess.DEVNULL, stderr=subprocess.PIPE
        )
    )
    time.sleep(1.5)
    for process in processes:
        if process.poll() is not None:
            error = process.stderr.read().decode(errors="replace").strip()
            log.say("MDNS", f"avahi-publish {' '.join(process.args[1:])} failed: {error}")
    log.say("MDNS", f"announcing {', '.join(n + '.local' for n in names)} -> {ip}, and _http._tcp \"{names[0]}\" on port 80")
    return processes


def local_ip():
    probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        probe.connect(("192.0.2.1", 9))
        return probe.getsockname()[0]
    finally:
        probe.close()


def keyboard(log, xenia):
    help_text = "Enter: end the shot · a: abort · k: knob press · s N: force MA_STATUS · t N: temp · w N: tank · v: verbose polls · q: quit"
    log.say("KEYS", help_text)
    for line in sys.stdin:
        command, _, argument = line.strip().partition(" ")
        argument = argument.strip()
        if command == "":
            xenia.end(aborted=False)
        elif command == "a":
            xenia.end(aborted=True)
        elif command == "k":
            xenia.knob_press("keyboard")
        elif command in ("s", "t", "w"):
            try:
                value = float(argument) if argument else None
            except ValueError:
                log.say("KEYS", f"not a number: {argument}")
                continue
            with xenia.lock:
                if command == "s":
                    xenia.force_status = None if value is None else int(value)
                elif command == "t":
                    xenia.temp = 93.0 if value is None else value
                else:
                    xenia.tank = 1 if value is None else int(value)
            forced = "as the phase says" if xenia.force_status is None else xenia.force_status
            log.say("KEYS", f"MA_STATUS {forced}, BG_SENS_TEMP {xenia.temp}, TANK_LEVEL {xenia.tank}")
        elif command == "v":
            log.verbose_polls = not log.verbose_polls
            log.say("KEYS", f"polls {'in full' if log.verbose_polls else 'one line each'}")
        elif command == "q":
            break
        else:
            log.say("KEYS", help_text)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--port", type=int, default=80, help="the Xenia's HTTP port (default 80, the grinder's)")
    parser.add_argument("--ip", help="this computer's IP on the grinder's network (default: found)")
    parser.add_argument(
        "--shot-seconds", type=float, default=27.0, help="a shot ends by itself after this (default 27; 0: only by Enter)"
    )
    parser.add_argument("--serial", default="CS0FA4E000", help="MA_SN in the identity (10 characters)")
    parser.add_argument("--close", action="store_true", help="close the connection after every reply")
    parser.add_argument("--idle", type=float, default=60, help="close a connection idle this long (s, default 60)")
    parser.add_argument("--watch-ports", default="443,8080", help="other ports to log connections on ('' for none)")
    parser.add_argument("--no-mdns", action="store_true", help="don't listen to mDNS")
    parser.add_argument("--mdns-all", action="store_true", help="print every mDNS packet, not only the grinder's and the Xenia's names")
    parser.add_argument(
        "--announce", action="store_true", help="announce xenia-coffee.local and xenia.local, for the grinder's Scan"
    )
    parser.add_argument("--grinder", action="append", default=[], help="the grinder's IP, its mDNS packets are all shown")
    parser.add_argument("--log", help="JSON-lines file (default fake_xenia_<date>_<time>.jsonl here, '' for none)")
    args = parser.parse_args()

    ip = args.ip or local_ip()
    path = args.log if args.log is not None else datetime.datetime.now().strftime("fake_xenia_%Y-%m-%d_%H%M%S.jsonl")
    log = Log(path)
    if path and os.environ.get("SUDO_UID"):
        # run with sudo: the file stays the user's
        os.chown(path, int(os.environ["SUDO_UID"]), int(os.environ.get("SUDO_GID", "-1")))
    xenia = Xenia(log, args.serial, args.shot_seconds)
    grinders = set(args.grinder)

    try:
        server = HttpServer(log, xenia, args.port, args.close, args.idle, grinders)
    except PermissionError:
        sys.exit(f"Port {args.port} needs root: sudo python3 {sys.argv[0]} ...")
    except OSError as error:
        sys.exit(f"Port {args.port}: {error}. Is something else (a web server, gbs-anywhere) using it?")
    log.say("XENIA", f"playing a Xenia at http://{ip}:{args.port}  ->  grinder: Edit Host {ip}" + ("" if args.port == 80 else "  (the grinder only asks port 80!)"))
    if path:
        log.say("XENIA", f"logging to {os.path.abspath(path)}")
    log.event("start", ip=ip, port=args.port, args=vars(args))
    threading.Thread(target=server.serve, daemon=True).start()

    for port in filter(None, (p.strip() for p in args.watch_ports.split(","))):
        try:
            threading.Thread(target=watch_port, args=(log, listen_tcp(int(port)), port), daemon=True).start()
            log.say("TCP", f"also logging connections to port {port}")
        except OSError as error:
            log.say("TCP", f"can't watch port {port}: {error}")

    if not args.no_mdns:
        try:
            sock = open_mdns(ip)
            threading.Thread(target=sniff_mdns, args=(log, sock, ip, args.mdns_all, grinders), daemon=True).start()
        except OSError as error:
            log.say("MDNS", f"can't listen to mDNS: {error}. Try: sudo tcpdump -ni any udp port 5353")
    publishers = announce(log, ip, ["xenia-coffee", "xenia"]) if args.announce else []

    try:
        if sys.stdin.isatty():
            keyboard(log, xenia)
        else:
            while True:
                time.sleep(3600)
    except KeyboardInterrupt:
        pass
    finally:
        for process in publishers:
            process.terminate()
        log.say("XENIA", f"stopped after {server.polls} poll(s)")


if __name__ == "__main__":
    main()
