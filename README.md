# Coffee-Scale

[![Release Build](https://github.com/orlopau/coffee-scale/actions/workflows/pio_deploy.yaml/badge.svg)](https://github.com/orlopau/coffee-scale/actions/workflows/pio_deploy.yaml)
[![CI Tests](https://github.com/orlopau/coffee-scale/actions/workflows/pio_test.yaml/badge.svg?branch=master)](https://github.com/orlopau/coffee-scale/actions/workflows/pio_test.yaml)

---

This project contains the schematics, code and 3D models for a coffee scale based on an ESP32.

<img src="docs/img/3d.png" width="500">
 
## Features
* OTA updates via WiFi
* Recipe-based brewing
* Auto averaging of weight values: more values are used to determine weight when weight is relatively constant
* Brew timer
* USB-C charging
* Interchangeable Load Cell
* 3D-printable case
* Bluetooth and Wi-Fi connectivity

## Recipe-Based Brewing
The scale can be used to brew coffee using a step-by-step recipe. 
The recipe is stored on the scale, and can be adjusted while brewing.

Possible adjustments include:
- Changing the amount of coffee
- Changing the brew ratio
- Other adjustments based on the chosen recipe

## Development: Updating From Your Computer
During development, the scale can install a build straight from your computer over Wi-Fi, without USB and without a GitHub release.

1. In `code/`, run `pio run -t serve`. This builds the firmware and serves it on your network. It is found by the scale via mDNS, so your computer's IP does not matter. The first run installs the Python package `zeroconf`.
2. Hold the button while switching on the scale to open the updater. It connects to the Wi-Fi saved in the updater.
3. On the language screen, **long-press** the button instead of clicking. The scale searches for your computer and installs the build.

The server keeps running, so for the next build repeat steps 2 and 3 (`pio run -t serve` rebuilds when restarted). If the scale already runs the served build, it shows "No update available". Your computer and the scale must be on the same network, and the firewall must allow incoming connections for the server.
