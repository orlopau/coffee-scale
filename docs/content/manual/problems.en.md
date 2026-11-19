---
title: Problems
weight: 9
---

## The weight is wrong

- Switch the scale on with nothing on the platform. It sets its zero at the start.
- Tare it: turn the knob a little.
- If it's still wrong, [calibrate](../calibration/) it with a known weight.

## The weight jumps around

Put the scale on a firm, level surface, away from vibrations. If it still jumps, raise **Weight Averaging Threshold** in the [settings](../settings/).

## It tares when I don't want it to

Lift-to-tare or auto-tare may be doing it. Choose a firmer **Lift-to-Tare** level, or turn it off, and check your **Auto-Tare Weights** in the [settings](../settings/).

## A shot isn't detected

- Is **Shot Detection** on in the [settings](../settings/)? It's off by default.
- Tare the cup before the shot. The scale only watches for a shot after a tare.
- A bump while the shot starts can hide it. Click to start the timer by hand.

## Can't read calibration

{{< screen name="error_settings_nvs" caption="Can't read calibration" >}}

At the start, the scale couldn't read its calibration and settings from its memory. It weighs, but not correctly. Click to go on, then [calibrate](../calibration/) it and check the [settings](../settings/). If this shows at every start, the scale's memory may be faulty.

## Wi-Fi

| The scale says | What to do |
|---|---|
| **Wrong password. Please try again.** | Enter the password again on the setup page; it's case-sensitive. |
| **Network not found. Please try again.** | Is the Wi-Fi on and in reach? The scale only works with 2.4 GHz Wi-Fi. |
| **Connection failed. Please try again.** | Try again; move the scale closer to the router. |
| **Wi-Fi doesn't start.** | Switch the scale off and on again. |

{{< screen name="wifi_wrong_password" caption="Wrong password. Please try again." >}}

## Updates

| The scale says | What to do |
|---|---|
| **No update available.** | The scale already runs the newest firmware. |
| **Update failed.** | Check that Wi-Fi works and try again; with an automatic update, the scale asks again at the next start. |
| **Update failed. Previous version restored.** | The new firmware didn't start properly, so the scale went back to the one before. Nothing to do; that version isn't offered again. |
