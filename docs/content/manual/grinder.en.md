---
title: Mahlkönig grinder
weight: 4
---

A Mahlkönig E64 WS dials itself in with Grind-by-Sync (GbS): after each shot it compares the shot's time with your recipe's brew time and grinds finer or coarser. For that it needs an espresso machine that tells it the time, normally Mahlkönig's Xenia. The scale can take the Xenia's place, with any espresso machine. All it needs is your Wi-Fi, no other device.

{{< screen name="espresso_grinder" caption="After the knob press: the recipe's 40.0 g as the target, the grinder's mark top right" >}}

## Setting it up

{{% steps %}}

### Connect the scale to your Wi-Fi

The scale and the grinder have to be on the same network. See [Setting up Wi-Fi](../getting-started/#setting-up-wi-fi). If the scale has no Wi-Fi yet, it opens the Wi-Fi setup by itself when you turn the setting on and leave the settings.

### Turn the setting on

In the [settings](../settings/), set **Mahlkönig grinder** to **On**. While you change it, the scale shows its name and address below it. You'll need them on the grinder.

{{< screen name="settings_grinder_on_connected" caption="On, with the scale's name and address" >}}

The setting starts when you leave the settings. If it was off before, the address shows only after that, so leave the settings and open them again to see it.

{{< screen name="settings_grinder_on_not_active" caption="Turned on just now: active after leaving the settings" >}}

### Enable the Xenia on the grinder

On the grinder, go to **Connectivity** → **Machine To Machine** and turn on **Enable Xenia**.

### Let the grinder find the scale

Go to **Configuration** → **Scan** and pick the scale from the list. If it isn't in the list, go to **Configuration** → **Edit Host** and enter `coffee-scale.local`. If that doesn't work either, enter the address the scale shows, e.g. `192.168.1.42`.

### Look for the mark

Within a few seconds a small square appears in the top right corner of the scale's display. It shows that the grinder is talking to the scale. It stays there as long as the grinder asks the scale every few seconds.

{{% /steps %}}

## Pulling a shot

{{% steps %}}

### Grind

Grind with a GbS recipe on the grinder, as usual. The grinder sends the recipe's brew weight to the scale.

### Put the cup on the scale

The scale should be in the scale mode or on the espresso screen.

### Press the grinder's knob

The scale switches to the espresso screen, tares, and takes the recipe's brew weight as the target.

### Pull the shot

With [shot detection](../espresso/#shot-detection) on, just start the pump. Without it, **click** when you start the pump, as usual.

### The grinder adjusts

When the shot ends, the scale sends its time to the grinder, and the grinder adjusts its grind for the next shot.

{{% /steps %}}

## The time the grinder gets

A Xenia stops its pump when the cup reaches the brew weight, so the grinder expects **the time until the cup holds the brew weight**. The scale sends exactly that, whenever you stop the pump. If you stop a little late, the grinder still gets the right time and doesn't grind coarser for no reason.

- The time starts where the espresso timer starts: at the pump start with shot detection, at your click without it.
- If the cup reaches the brew weight with the drips, within 3 seconds after the pump stopped, the shot still counts, with the time until the pump stop.
- The weight in the cup is sent too. The grinder keeps it, but adjusts from the time only.

### Shots the grinder skips

Anything that isn't a full shot goes to the grinder as cancelled, and the grinder doesn't adjust from it:

- the shot ends before the cup reaches the brew weight,
- you leave the espresso screen before the shot ended, or open the list of modes,
- no shot starts within 3 minutes of the knob press,
- the knob is pressed while recipes, calibration, the settings or the list of modes are open, while the scale mode's timer runs, or while a shot runs. The scale stays where it is.

The grinder itself also ignores shots of 10 seconds or less, or over 80 seconds.

## Good to know

- **The target stays yours.** You can turn the knob to change the target the grinder set, as with one you set yourself. The grinder still gets the time when the cup reached the first target.
- **Only knob presses count.** A shot you pull without pressing the grinder's knob is never sent to the grinder.
- **Switch the scale on before you grind.** While it's off, the grinder can't reach it.
- **The battery runs down faster.** With the setting on, the scale stays on Wi-Fi as long as it's on. Turn the setting off when you don't use the grinder. If your Wi-Fi isn't in reach, the scale tries again after a few seconds, then less and less often, at most once a minute.
- **During the Wi-Fi setup** the grinder can't reach the scale; afterwards the mark comes back by itself. If the scale is now on another network, its address may have changed: if you entered the address on the grinder, enter the new one.

## If it doesn't work

| What you see | What to do |
|---|---|
| The setting shows **Not connected** | The scale isn't on your Wi-Fi. Check that the network is in reach, or set it up again under **Wi-Fi** in the settings. |
| No mark in the top right corner | The grinder doesn't reach the scale. Check that both are on the same network and that **Enable Xenia** is on. Under **Edit Host**, try the address the scale shows instead of `coffee-scale.local`. |
| The knob press does nothing on the scale | The scale has to be in the scale mode or on the espresso screen, with no shot running. Elsewhere the grinder skips the shot. |
| The grinder doesn't adjust | It skips cancelled shots and shots under 10 or over 80 seconds. Check that the cup reached the brew weight. |
