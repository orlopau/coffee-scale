# Roadmap

What happens next on the scale, in the order we agreed on. Each phase builds on the one before.

Status: `[x]` done · `[~]` in progress · `[ ]` open

## 01 Foundation

Cleanup and tooling before new features.

- [x] **Display refactor** _(Firmware)_
  All modes and the updater draw themselves on a canvas; frames are only sent when something changed.
- [x] **Dev updates from the laptop** _(Firmware, Tooling)_
  `pio run -t serve`, then long-press in the updater. The scale finds the laptop via mDNS.
- [x] **ESP32 build in CI** _(Tooling)_
  Every push builds the ESP32 firmware in English and German, next to the native tests and the emulator, so errors show up right away, not at the next release.
- [x] **Emulator and screenshot tests** _(Tooling)_
  `pio run -e emulator -t exec` shows the scale in a desktop window; arrow keys and Enter replace the rotary encoder. Simulated load cell at 10 Hz, virtual clock, and every screen is compared against a PNG in CI.
- [x] **Raw data recorder** _(Firmware, Tooling)_
  The scale records readings and uploads them to the dev server; native tests replay them. Next: record real taps, shots and cups.
- [x] **Fix the inputs** _(Firmware)_
  The ESP32's pulse counter counts the rotary encoder instead of an interrupt; clicks go into a queue as events. Fixes lost or doubled clicks and possible crashes while writing to flash.
- [x] **Load cell task and weight pipeline** _(Firmware)_
  A dedicated task reads the load cell via the data-ready pin and timestamps every reading. Calibration, tare, filter and flow are separate stages. Shot, tap and auto-tare detection read this stream independently of the mode.
- [x] **Settings in NVS** _(Firmware)_
  Settings and calibration stored under keys in NVS, via the ESP-IDF API, instead of at fixed EEPROM addresses.
- [x] **Own Wi-Fi setup instead of AutoConnect, and safe updates** _(Firmware)_
  Set up from the phone, no app: the scale opens its own access point and shows a Wi-Fi QR code; the camera joins it and the setup page opens. Asked once on the first start, then in the settings menu. Built on the ESP-IDF API, so it carries over. Updates check GitHub's certificate against the CA bundle instead of `setInsecure()`, and a firmware that crashes in its first seconds rolls back to the previous one. Core 2.0.17 already has both, so the move to Core 3.x waits.
- [ ] **Automatic updates** _(Firmware)_
  Every once in a while the scale connects to the saved Wi-Fi by itself, checks whether a newer firmware is out, and installs it. Needs the saved network, certificate checks and rollback from the item before.
- [x] **Zero tracking** _(Firmware)_
  When the scale is empty and steady, it slowly pulls small deviations around zero back to zero, e.g. as it warms up. Paused during shots and recipes so slow first drops aren't tracked away.

## 02 Detection

The scale understands what happens on it.

- [x] **Lift-to-tare** _(Firmware)_
  Sliding a finger up the side of the platform, lifting it a little, tares. Taps can't be told apart at 10 readings per second, lifts can: nothing else on the counter dips below the weight and comes right back, see [ADR 1](adr/0001-tare-gesture.md). How firmly you have to lift is adjustable. Uses the unsmoothed readings and is locked during a shot.
- [ ] **Shot detection** _(Firmware)_
  Starts at the first drop and ends when the flow drops below 0.1 g/s. Setting down the cup, bumps and false starts are ignored. With an adjustable offset between pump start and first drop.
- [ ] **Shot graph on the display** _(Firmware)_
  Live curve with target line and flow in g/s. After the shot, a summary with yield, time and ratio.

## 03 Shot log & web app

Record every shot and analyze it.

- [ ] **Shot log on the scale** _(Firmware)_
  Every detected shot is stored in flash, in LittleFS on the data partition: time, dose, yield, duration and the curve at 5 Hz, under 1 KB per shot. The time comes via NTP once Wi-Fi is on.
- [ ] **Shot server** _(Server)_
  A container behind your Traefik stores the shots in SQLite, separated by scale. Reachable on its own subdomain so the address in the firmware survives a server move. Later it also computes the grinder model.
- [ ] **The scale is the account** _(Server, Firmware)_
  No accounts: on first start the scale generates a secret key and uses it to sign in to the server. It shows access to the web app as a QR code, so whoever holds the scale has access. The link can be renewed on the scale.
- [ ] **Upload from the scale** _(Firmware)_
  After each shot the scale briefly turns on Wi-Fi and uploads it via HTTPS. Until that succeeds, the shot stays in flash.
- [ ] **Web app** _(Web)_
  Runs on the server and works on any phone: curves, history and notes per shot on bean, grind setting and taste.
- [ ] **Claude connector** _(Server)_
  An MCP endpoint on the server. Everyone adds it to Claude as their own connector, with their scale's access token, and can then ask about their shots in any chat.
- [ ] **Flash firmware in the browser** _(Web, Tooling)_
  ESP Web Tools on the same page: install firmware over USB straight from the browser, without PlatformIO.

## Ideas for later

Collected, but not yet planned.

- **The scale learns the grinder** _(Firmware, Web)_
  From past shots the scale learns, per bean, how much one click changes the shot time. Then it tells you how many clicks finer or coarser to go. Needs shot detection and the shot log.
- **Compare shots between scales** _(Web)_
  People using the same beans share a reference shot with dose, yield, time and curve. The other scale takes it as a target and, using the grinder model, shows how to get there.
- **Pump start via smart plug** _(Firmware)_
  A smart plug with power metering on the machine detects the pump start. Together with the first drop from the scale, that gives the real shot time and the preinfusion duration.
- **Grind-by-weight for the Philos** _(Firmware, Hardware)_
  A smart plug (Shelly, 16 A) switches off the grinder just before the target weight. The overrun offset learns from every grind.
- **Live weight via Bluetooth** _(Firmware)_
  The Decent Scale protocol, so apps like Beanconqueror can use the scale directly.
- **Dose sets the target weight** _(Firmware)_
  The measured dose sets the espresso target automatically, e.g. 18.2 g × 2 = 36.4 g.
- **Container detection** _(Firmware)_
  The weight of the container set down picks the mode: portafilter opens espresso, V60 opens your recipe.
- **Auto-sleep** _(Firmware)_
  Deep sleep after a few minutes without use, wake via the rotary encoder.
- **Arduino Core 3.x** _(Firmware)_
  IDF 5 underneath. The official PlatformIO platform still ships Core 2.0.17, Core 3.x only comes through the pioarduino fork. Waits until the platform supports it, or for the move to plain ESP-IDF.
