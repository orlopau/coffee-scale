---
title: Updates
weight: 8
---

New firmware brings new features and fixes. The scale finds it by itself once [Wi-Fi is set up](../getting-started/#setting-up-wi-fi).

## Automatic updates

Every few starts, the scale looks for a new firmware in the background while you weigh. You won't notice it. When it finds one, it asks the next time you switch it on:

{{< screen name="updater_ask_yes" caption="Update to v10.10.10?" >}}

- **Yes** installs the update and restarts the scale. The display shows the progress; leave the scale on meanwhile.
- **Later** skips it for now. The scale asks again at the next start.

{{< screen name="updater_progress" caption="Updating: 42.17%" >}}

## Updating by hand

You can also install the newest firmware right away:

{{% steps %}}

### Hold the button while switching on

Keep the knob pressed while you switch the scale on, then let go.

### Wait for Wi-Fi

The scale connects to your Wi-Fi and shows **WiFi connected.**. If it can't, it opens the [Wi-Fi setup](../getting-started/#setting-up-wi-fi).

### Choose the language

**Turn** to the language you want the scale in and **click**. The scale downloads the newest firmware in that language, installs it and restarts.

{{< screen name="updater_language_1" caption="Choose Language" >}}

{{% /steps %}}

If the scale already runs the newest firmware, it says **No update available.**.

## If an update goes wrong

If a new firmware doesn't start properly, the scale goes back to the one it had before by itself. It tells you so at the next start, and doesn't offer that version again. Click to go on.

{{< screen name="updater_rolled_back" caption="Update failed. Previous version restored." >}}
