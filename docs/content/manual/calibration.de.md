---
title: Kalibrieren
weight: 7
---

Beim Kalibrieren lernt die Waage, was 100 g wiegen. Kalibriere sie einmal, wenn sie neu ist, und wieder, wenn sie falsch wiegt, z. B. nach dem Tausch der Wägezelle, oder wenn die Waage meldet, dass sie ihre [Kalibrierung nicht lesen kann](../problems/#kalibrierung-nicht-lesbar).

Du brauchst etwas, das **genau 100 g** wiegt, z. B. ein Kalibriergewicht. Das Ergebnis ist nur so genau wie dieses Gewicht.

{{< callout type="info" >}}
Die Kalibrierung spricht bisher nur Englisch, auch in der deutschen Firmware. Die Texte unten stehen deshalb auf Englisch, so wie die Waage sie zeigt.
{{< /callout >}}

{{% steps %}}

### Waage leeren

Öffne **Kalibrierung** in der Liste der Modi. Nimm alles von der Plattform und **klick**.

{{< screen name="calibration_begin" caption="Starting calibration." >}}

### 100 g auflegen

Leg das 100-g-Gewicht auf die Plattform, warte, bis es ruhig liegt, und **klick**.

{{< screen name="calibration_add_weight" caption="Add 100 g to scale." >}}

### Warten

Die Waage nimmt 100 Messwerte, das dauert etwa zehn Sekunden. Fass sie währenddessen nicht an.

{{< screen name="calibration_calibrating" caption="Calibrating..." >}}

### Fertig

**Calibration complete.** Die neue Kalibrierung ist gespeichert. **Lange drücken**, um in einen anderen Modus zu wechseln.

{{< screen name="calibration_end" caption="Calibration complete." >}}

{{% /steps %}}

Während der Kalibrierung zeigen die Bildschirme ein paar Zahlen zur Kontrolle: `Raw` ist, was die Wägezelle gerade misst, mit ihren Messwerten pro Sekunde, `Tare` ihr Wert ohne Last, `Diff` der Unterschied. Ändert sich `Raw` nicht, wenn du das Gewicht auflegst, ist die Wägezelle nicht richtig angeschlossen.

{{< callout type="warning" >}}
Hat sich der Rohwert gar nicht geändert, meldet die Waage **Calibration failed: raw value unchanged. Not saved.** und behält ihre alte Kalibrierung. Prüfe, ob das Gewicht auf der Plattform liegt, und versuch es nochmal.
{{< /callout >}}

{{< screen name="calibration_unchanged" caption="Calibration failed: raw value unchanged." >}}
