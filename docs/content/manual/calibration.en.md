---
title: Calibration
weight: 6
---

Calibration teaches the scale what 100 g weighs. Calibrate it once when it's new, and again if it weighs wrong, e.g. after changing the load cell, or if the scale says it [can't read its calibration](../problems/#cant-read-calibration).

You need something that weighs **exactly 100 g**, e.g. a calibration weight. The result is only as exact as that weight.

{{% steps %}}

### Empty the scale

Open **Calibration** from the list of modes. Take everything off the platform and **click**.

{{< screen name="calibration_begin" caption="Starting calibration." >}}

### Put on 100 g

Put the 100 g weight on the platform, wait until it's still and **click**.

{{< screen name="calibration_add_weight" caption="Add 100 g to scale." >}}

### Wait

The scale takes 100 readings, which takes about ten seconds. Don't touch it meanwhile.

{{< screen name="calibration_calibrating" caption="Calibrating..." >}}

### Done

**Calibration complete.** The new calibration is saved. **Long-press** to go to another mode.

{{< screen name="calibration_end" caption="Calibration complete." >}}

{{% /steps %}}

While calibrating, the screens show some numbers for checking: `Raw` is what the load cell reads right now, with its readings per second, `Tare` its reading when empty, `Diff` the difference. If `Raw` doesn't change when you put the weight on, the load cell isn't connected properly.

{{< callout type="warning" >}}
If the raw value didn't change at all, the scale says **Calibration failed: raw value unchanged. Not saved.** and keeps its old calibration. Check that the weight is on the platform and try again.
{{< /callout >}}

{{< screen name="calibration_unchanged" caption="Calibration failed: raw value unchanged." >}}
