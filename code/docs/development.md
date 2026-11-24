# Development

How to work on the firmware: installing builds from your computer, trying the automatic updates, recording the load cell, the emulator and the screenshot tests. The commands run in `code/`. For using the scale, see the [manual](https://coffee-scale.orlopau.dev); for what comes next, the [roadmap](roadmap.md).

## Updating From Your Computer
During development, the scale can install a build straight from your computer over Wi-Fi, without USB and without a GitHub release.

1. In `code/`, run `pio run -t serve`. This builds the firmware and serves it on your network. It is found by the scale via mDNS, so your computer's IP does not matter. The first run installs the Python package `zeroconf`.
2. Hold the button while switching on the scale to open the updater. It connects to the Wi-Fi saved in the updater.
3. On the language screen, **long-press** the button instead of clicking. The scale searches for your computer and installs the build. (If it finds no server, it opens the recorder, see below.)

The server keeps running, so for the next build repeat steps 2 and 3 (`pio run -t serve` rebuilds when restarted). If the scale already runs the served build, it shows "No update available". Your computer and the scale must be on the same network, and the firewall must allow incoming connections for the server.

## Watching the Log
To see the scale's log without a serial cable, build the log firmware. It is the normal firmware plus a task that sends every log line to the dev server over Wi-Fi.

1. In `code/`, run `pio run -e esp32dev-log -t serve` and long-press in the updater, as for any update above.
2. The scale installs the build and restarts. The terminal of `serve` now prints its log from boot on, one line per log line, with the time and the scale's address: `12:03:44 [192.168.1.37] [  5123][I][main.cpp:148] setup(): [MAIN] Tared!`. ESP-IDF's own lines (mostly Wi-Fi errors) look like `E (5123) wifi: ...`.
3. To go back to the normal firmware, run `pio run -t serve` and long-press again.

Every mode of the dev server prints the log, also `record` and `--release`. `--no-log` turns it off, e.g. `python scripts/dev_server.py .pio/build/esp32dev/firmware.bin --no-log`. Then the server doesn't tell the scale where to send it.

- **What arrives:** the lines of the app and of ESP-IDF, without color codes. Nothing is saved on the scale: no log after the fact, no log of the last crash. UDP can lose a line on a bad network.
- **Boot:** lines from before Wi-Fi was up wait in a buffer in RAM until the scale has found the dev server. If the buffer fills up first, further lines are dropped, and once there is room again a line `net_log: N lines dropped` marks the gap.
- **Stack:** once a minute the log firmware logs the stack its tasks have left, e.g. `Stack left (bytes): net_log 1234, button 567, loadcell 890`.
- **Wi-Fi stays on** in the log firmware, so it uses more battery. Install it only while testing, and never in a release.
- **Restarted dev server:** UDP doesn't tell the scale that the server is gone. Restart the scale, then it finds the new one.
- **Wi-Fi setup:** while a saved network is unreachable, the log firmware's search for the dev server keeps trying to connect, which can slow the scale's Wi-Fi setup down. To redo the setup, switch back to the normal firmware first.

## Playing the Xenia From the Computer
To see what the Mahlkönig E64 WS sends without the scale in between, `scripts/fake_xenia.py` plays the Xenia on the computer. It answers like the scale (`src/grinder/xenia.cpp`) and logs every connection, request, header, body and reply, requests nobody knows, bytes that aren't HTTP, and the mDNS questions on the network. Everything also goes to a JSON-lines file.

1. Turn the scale's "Mahlkönig grinder" setting off, so the grinder can't reach the scale.
2. In `code/`, run `sudo python3 scripts/fake_xenia.py` (port 80 needs root; the grinder asks no other port). It prints the computer's IP.
3. On the grinder, set that IP under Connectivity → Machine To Machine → Configuration → Edit Host. Polls show up every 2 s.
4. Grind a GbS recipe and press the knob. A shot ends by itself after `--shot-seconds` (27 s), or on Enter. `a` aborts it, `s N` forces `MA_STATUS`, `t N` and `w N` the temperature and tank, `v` shows every poll in full.

`--announce` also answers as `xenia-coffee.local` and `xenia.local`, for the grinder's Scan. `--help` lists the rest.

## Trying Automatic Updates
The scale's automatic updates read GitHub's releases. To try them without publishing one, the dev server can pose as a release, and a build can claim to be an older version.

- **Against GitHub:** build with an older version, e.g. `FIRMWARE_VERSION=v1.8.0 pio run -t upload`. The scale then sees the latest release as newer.
- **Against your computer:** run `python scripts/dev_server.py .pio/build/esp32dev/firmware.bin --release v9.9.9 --port 8000` in `code/`. It prints the `RELEASES_URL` to build the scale with, e.g. `RELEASES_URL=http://192.168.1.20:8000/releases pio run -t upload`. Build the firmware to serve with the same `RELEASES_URL`, so it keeps asking your computer. The served build reports the last tag as its version, not `--release`, so after installing it the scale still finds v9.9.9 at every check; build it with `FIRMWARE_VERSION=v9.9.9` too to see the scale stay quiet once it's up to date.

A check runs at the first start of a new firmware if Wi-Fi is set up, then every 5th start; the serial log (`AUTO_UPDATE`) says when.

## Recording the Load Cell
The scale can record the raw readings of its load cell, e.g. of taps or an espresso shot, and save them in its flash. The recordings are test data: native tests replay them with `RecordedScale` (`code/src/native/recorded_scale.h`). You don't need your computer while recording.

1. Install a build with the recorder on the scale, see above.
2. Open the updater and long-press on the language screen, as for an update. If no server is found, the scale opens the recorder by itself. It shows how many recordings are saved, and "No server".

**Click** to start a recording. While recording, the scale shows the weight, the recording time, the time left and the number of markers. **Click** again to set a marker, e.g. when the pump starts. **Turn** the knob to tare the shown weight, e.g. after putting the cup on; while recording, the tare is saved in the recording too. **Long-press** to save the recording in the scale's flash; the scale then waits for the click that starts the next one. A recording takes at most 5 minutes, then the scale waits for the long-press. About 50 recordings fit. If saving fails, the recording is kept and the next long-press tries again. A recording that wasn't saved is lost when the scale is switched off.

With the log firmware, `record` prints the scale's log too, see above. To get the recordings to your computer, run `pio run -t record` in `code/`. It does not build anything, it waits for recordings. Only one of `serve` and `record` can run at a time. Open the recorder as above; the scale now finds the server and no longer says "No server". **Long-press** while idle to send all saved recordings, oldest first. Each one is deleted from the scale after it arrived, so if sending fails, the rest stays and the next long-press tries again.

The computer saves each recording as `code/test/recordings/<date>_<time>.csv`. Give the useful ones a name that says what they show, commit them, and delete the rest. A recording looks like this, with `ms` counted from its start on the scale's clock:

```
# coffee-scale recording
# firmware: v1.9.0
# grams_per_count: 0.002381
ms,event,value
0,sample,84012
101,sample,84020
1234,click,
1500,tare,
```

`grams_per_count` is the calibration of the scale that recorded, so tests can convert the raw readings to grams. A sample's `value` is the HX711's raw reading, signed. The samples stay raw after a tare, a `tare` row only says when it happened (`RecordedScale::tares()`). Firmware up to v1.9.0 recorded it unsigned (16540413 instead of -236803), and the replay converts those.

## Emulator
The emulator runs the firmware on your computer, in a window that shows the scale's display. It needs SDL2 (`sudo apt install libsdl2-dev` on Debian/Ubuntu).

```
cd code
pio run -e emulator -t exec
```

With `COMPILE_LANG=de` in front, it runs in German. The load cell is simulated, and the keyboard replaces the encoder:

| Key | Action |
|---|---|
| Left / Right | turn the encoder |
| Enter, Space | press the encoder, hold for a long press |
| Up / Down | put 1 g on the scale or take it off, with Shift 0.1 g |
| PageUp / PageDown | 10 g on or off |
| Home | 100 g, e.g. for the calibration |
| 0, Backspace | clear the scale |
| F | pour on/off, 2 g/s |
| S | espresso shot: 6 s pre-infusion, then about 38 g in 33 s |
| L | lift the platform for a moment (lift-to-tare) |
| N | noise on/off |
| - / + | slower/faster time, up to 20x (1x while the button is held) |
| P | save a screenshot to the current directory |
| Esc, Q | quit |

The window title shows the weight on the scale. The frame around the display lights up while the buzzer sounds. Settings are kept in memory only, and the boot splash screen and the updater only run on the device.

## Screenshot Tests
The native tests render every screen through u8g2, set up like the scale's display, and compare it with the reference images in `code/test/native/test_screenshots/screens/en/`. The env `native_de` renders them in German and compares them with `screens/de/`. A screen that changes fails the tests, and its new image is written to `code/test/native/test_screenshots/failed/en/` or `failed/de/` (on GitHub, attached to the CI run as `failed-screenshots`).

After changing a screen on purpose, accept the new images in both languages and commit them with the change:

```
cd code
UPDATE_SCREENSHOTS=1 pio test -e native -f native/test_screenshots
UPDATE_SCREENSHOTS=1 pio test -e native_de
```

The diff of the pull request then shows each changed screen before and after. The [manual](https://coffee-scale.orlopau.dev) shows these images, so a changed screen changes the manual too, after the next push to master.

## Manual
The manual's source is in `docs/` at the top of the repository: a [Hugo](https://gohugo.io) site with the [Hextra](https://github.com/imfing/hextra) theme as a git submodule, pages as `docs/content/manual/<page>.en.md` and `<page>.de.md`. Screens go into a page with `{{< screen name="scale_running" caption="..." >}}`, the name being an image in `screens/en/` and `screens/de/`; a name without an image fails the build.

To preview it, get the theme once, then run Hugo (extended, 0.146 or newer) in `docs/`:

```
git submodule update --init
cd docs
hugo server
```

The changelog page shows [CHANGELOG.md](../../CHANGELOG.md) from the top of the repository.

On a push to master that changes `docs/`, the screens or `CHANGELOG.md`, `.github/workflows/hugo.yml` publishes it to GitHub Pages.

## Changelog and Releasing
[CHANGELOG.md](../../CHANGELOG.md) follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), in English. Every change people using the scale would notice adds a line under `## [Unreleased]` in the same commit, in Added, Changed, Deprecated, Removed, Fixed or Security, written for them rather than for developers.

A release is a pushed `firmware-vX.Y.Z` tag: `.github/workflows/pio_deploy.yaml` builds the English and German firmware and attaches them to the GitHub release, where the scales find them. Before that, in one commit on master:

1. Rename `## [Unreleased]` to `## [X.Y.Z] - YYYY-MM-DD` and put a new, empty `## [Unreleased]` above it.
2. At the bottom, point `[unreleased]` at `firmware-vX.Y.Z...HEAD` and add `[X.Y.Z]: https://github.com/orlopau/coffee-scale/compare/<previous tag>...firmware-vX.Y.Z`.

Push that commit to master, then tag it with an annotated tag and push the tag:

```
git tag -a firmware-vX.Y.Z -m "Firmware vX.Y.Z"
git push origin firmware-vX.Y.Z
```

It must be annotated (`-a`): the build finds its version with `git describe`, which skips plain tags, so a plain tag gives a firmware that calls itself the previous version and offers itself as an update forever. The release build stops before building if `CHANGELOG.md` has no `## [X.Y.Z] - YYYY-MM-DD` for the tag; then add it, delete the tag (`git tag -d firmware-vX.Y.Z && git push origin :refs/tags/firmware-vX.Y.Z`) and tag again.
