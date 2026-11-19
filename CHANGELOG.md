# Changelog

All notable changes to the coffee scale's firmware are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Shot detection: the scale notices an espresso shot by itself. In the scale mode it switches to the espresso screen, on the espresso screen it starts the shot. The time counts from the pump start (the first drop minus the new Shot Offset setting, 7.0 s by default) and stops when the pump stops. Turn it on under Settings → Shot Detection; it is off by default.
- When a shot ends, the espresso screen shows the whole shot: its curve, the last shot's curve dotted behind it, the time and the yield. While a shot runs, it shows the flow in g/s.
- The calibration screen shows the load cell's raw reading and how many readings arrive per second, so a broken load cell is visible right away.
- Mahlkönig grinder: the scale takes the place of the Xenia espresso machine, so a Mahlkönig E64 WS dials itself in with Grind-by-Sync. A press on the grinder's knob opens the espresso screen with the recipe's brew weight as the target, and the grinder gets the time until the cup reached it, however late the pump stops. Turn it on under Settings → Mahlkönig grinder; it is off by default and keeps the scale on Wi-Fi while it is on. The manual's Mahlkönig grinder page shows how to set it up.
- The manual, in English and German, at [coffee-scale.orlopau.dev](https://coffee-scale.orlopau.dev), with this changelog.

### Fixed
- A calibration where the raw reading didn't change is no longer saved; the scale used to store an infinite calibration then.

## [1.10.0] - 2026-10-20

### Added
- The scale looks for a new firmware every 5th start and asks "Update to v1.x?" the next time you switch it on; Yes installs it, Later asks again.
- If a new firmware doesn't start properly, the scale goes back to the previous one, says so, and doesn't offer that version again.
- Wi-Fi is set up from the phone: scan the QR code on the scale and pick your network, at the first start or under Settings → Wi-Fi.
- Lift the platform slightly to tare; how firm a lift it needs is the new setting Lift-to-Tare (Off, Gentle, Normal, Firm).
- The empty scale stays at 0.0 g while it warms up, and holds its zero while a mode measures, so slow first drops aren't lost; the new setting Zero Tracking sets how far from zero it pulls back.
- If the calibration can't be read, the scale says so instead of weighing wrongly.

### Changed
- A single noisy reading no longer makes the weight jump for a second.
- Quick turns of the knob no longer skip steps, and clicks are no longer lost.
- The load cell is read as soon as it has a new value, so espresso times its readings more accurately.
- Updates check GitHub's certificate and download faster.
- The settings and the calibration are stored in a new way. Going back to v1.9.0 after this update, by hand or by a rollback, loses them, as v1.9.0 can't read them; calibrate again, or update again.

### Fixed
- Calibrating no longer leaves the scale tared with the old calibration (e.g. -83700 g).
- A cup already on the scale no longer stops an espresso shot right at the start.
- The result screen after calibrating shows the saved value instead of a random one.
- The German Wi-Fi text in the updater shows "ä" correctly and fits on the screen.
- The load cell's value is read signed, so the weight no longer jumps when the raw reading crosses zero.

## [1.9.0] - 2026-10-09

### Changed
- The display is only redrawn when something on it changed.

### Fixed
- The espresso progress bar was drawn full when the weight was slightly below zero after taring.

## [1.8.0] - 2025-02-07

### Added
- The setting Weight Averaging Threshold.

## [1.7.0] - 2024-12-14

### Added
- The setting Auto-Tare Tolerance.

## [1.6.0] - 2024-12-14

### Added
- Auto-tare.

## [1.5.1] - 2023-05-06

### Fixed
- Auto averaging didn't work with a negative calibration.

## [1.5.0] - 2023-05-06

### Added
- The calibration shows its result, and the calibration can be negative.
- The Kasuya V60 recipe advances by itself.
- A pour can be skipped before it has finished.

### Fixed
- The weight was scaled wrongly after setting a ratio and going back.

## [1.4.0] - 2023-01-29

### Added
- Espresso mode, which shows the estimated time left.

## [1.3.0] - 2023-01-21

### Added
- The firmware in several languages; the updater asks which one to install.

## [1.2.0] - 2023-01-11

### Added
- QR codes on the recipe summary.
- New recipes.

### Fixed
- The battery display updates at the right interval.

## [1.1.0] - 2023-01-10

### Added
- More recipes.
- Long texts, e.g. in recipes, wrap automatically.

### Changed
- A long click is detected differently.
- The firmware can use the whole flash.
- The first tare averages several readings instead of taking one.

### Fixed
- The battery display no longer flickers.

## [1.0.0] - 2023-01-09

### Added
- First release.

[unreleased]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.10.0...HEAD
[1.10.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.9.0...firmware-v1.10.0
[1.9.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.8.0...firmware-v1.9.0
[1.8.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.7.0...firmware-v1.8.0
[1.7.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.6.0...firmware-v1.7.0
[1.6.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.5.1...firmware-v1.6.0
[1.5.1]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.5.0...firmware-v1.5.1
[1.5.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.4.0...firmware-v1.5.0
[1.4.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.3.0...firmware-v1.4.0
[1.3.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.2.0...firmware-v1.3.0
[1.2.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.1.0...firmware-v1.2.0
[1.1.0]: https://github.com/orlopau/coffee-scale/compare/firmware-v1.0.0...firmware-v1.1.0
[1.0.0]: https://github.com/orlopau/coffee-scale/releases/tag/firmware-v1.0.0
