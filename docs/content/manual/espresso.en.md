---
title: Espresso
weight: 3
---

The espresso screen times a shot and shows how close it is to your target weight.

{{< screen name="espresso_brewing" caption="A shot running: 4.0 s in, about 3.2 s to go, 5.0 g/s, 20.0 g of 36.0 g" >}}

- **Top:** the time since the shot started. Once the scale can estimate when the shot reaches the target, the time still to go stands left of it, e.g. `-3.2s|4.0s`.
- **Middle:** the flow in grams per second, while the shot runs.
- **Bottom:** the weight in the cup and the target weight, with a bar that fills up.

## Pulling a shot by hand

{{% steps %}}

### Set the target

Open **Espresso** from the list of modes. **Turn** the knob to set the target weight, in steps of 0.1 g, from 1 g to 100 g. It starts at 36.0 g.

{{< screen name="espresso_idle" caption="Ready, with a target of 36.0 g" >}}

### Start

Put the cup on the scale and **click** when you start the pump. The scale tares and starts the timer.

### Stop

The timer stops by itself when the cup reaches the target weight, or when you **click** again.

{{% /steps %}}

## After the shot

When the shot ends, the scale shows its whole curve: the weight over time, the yield on a dashed line at its height, and the shot's time bottom right. The shot before is drawn dotted behind it, so you can compare.

{{< screen name="espresso_shot_with_last" caption="36.5 g in 28.0 s, the shot before dotted behind it" >}}

**Click** to go back to the espresso screen; the next click starts a new shot. Taring, e.g. by lifting the platform for the next cup, also goes back.

{{< callout type="info" >}}
With a Mahlkönig E64 WS, a press on the grinder's knob opens this screen with your recipe's weight as the target, and the grinder dials itself in from the shot. See [Mahlkönig grinder](../grinder/).
{{< /callout >}}

## Shot detection

With shot detection on, you don't need to click: the scale notices the shot by itself.

{{% steps %}}

### Tare the cup

Put the cup on the scale and tare it, in the scale mode or on the espresso screen.

### Pull the shot

Start the pump. When the first drops have flowed smoothly for a few seconds, the scale switches to the espresso screen (if it was in the scale mode) and starts the timer, counted from the pump start.

### Stop the pump

The timer stops at the moment the pump stops, not when the last drops land. A click stops it too.

{{% /steps %}}

{{< callout type="info" >}}
Shot detection is **off** by default. Turn it on in the [settings](../settings/), under **Shot Detection**.
{{< /callout >}}

Good to know:

- The scale only watches for a shot after a tare. Drips after a shot, or a cup set down, don't start one.
- Grinding or dosing beans into a tared cup doesn't start a shot: beans fall unevenly, a shot flows smoothly.
- If you bump the scale while the shot starts, it may not notice the shot. Then click to start the timer by hand.
- Recipes, calibration and the settings are never interrupted.
- The target weight doesn't stop a detected shot; only the pump stop or a click do.

### Shot offset

The scale can't see the pump, only the first drop, which comes a few seconds after the pump starts. So it starts the timer that many seconds earlier. **Shot Offset** in the settings sets those seconds, 7.0 by default. If your shot times seem too long or too short compared with your machine's, change it: time from pump start to the first drop on your machine, and use that.
