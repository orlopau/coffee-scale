---
title: Getting started
weight: 1
---

## The knob

You control the scale with one knob. It does three things:

- **Turn** it to move through lists and change values. While weighing, turning tares the scale.
- **Click** it to choose, start or stop.
- **Long-press** it (hold it for about a second) to go back, or to open the list of modes.

## Charging

The scale charges over USB-C. The battery level shows in the list of modes (long-press): an icon in the bottom left and the battery's voltage in the bottom right. While it charges, the icon shows a plug.

{{< screen name="mode_switcher_0" caption="The list of modes, with the battery in the bottom left" >}}

## Switching on

Switch the scale on with **nothing on the platform**: it sets its zero right at the start. Then it opens the scale mode.

Before that, it may ask a question:

- On the very first start, it asks whether to set up Wi-Fi, see below.
- When it has found a new firmware, it asks whether to install it, see [Updates](../updates/).

## Setting up Wi-Fi

The scale uses Wi-Fi to look for updates, and to talk to a [Mahlkönig grinder](../grinder/). You set it up from your phone, no app needed.

{{% steps %}}

### Say yes

On the first start the scale asks **Set up Wi-Fi?**. Turn to **Yes** and click. Later you find the same in the [settings](../settings/), under **Wi-Fi**.

{{< screen name="wifi_ask_yes" caption="Set up Wi-Fi?" >}}

### Scan the QR code

The scale opens its own Wi-Fi network and shows a QR code. Point your phone's camera at it and join the network. You can also join by hand: the network's name is on the display, the password is `coffee-scale`.

{{< screen name="wifi_join" caption="The scale's own network, with its QR code" >}}

### Choose your Wi-Fi

The setup page opens on your phone by itself. If it doesn't, open the browser and go to `http://192.168.4.1`. Choose your network, enter its password and tap **Connect**.

### Wait for "Connected."

The scale shows **Connecting...** and then **Connected.**. The page on your phone says so too, and you can close it.

{{< screen name="wifi_connected" caption="Connected." >}}

{{% /steps %}}

If the password was wrong or the network wasn't found, the scale says so and the setup page lets you try again. A click on the scale ends the setup without saving anything. More in [Problems](../problems/#wi-fi).

## Switching modes

**Long-press** to open the list of modes, **turn** to pick one and **click** to open it:

| Mode | What it's for |
|---|---|
| **Scale** | Weighing, with a timer. See [Weighing](../weighing/). |
| **Recipes** | Pour-over and Aeropress step by step. See [Recipes](../recipes/). |
| **Espresso** | Shots with a target weight. See [Espresso](../espresso/). |
| **Calibration** | Teaching the scale what 100 g is. See [Calibration](../calibration/). |
| **Settings** | See [Settings](../settings/). |
