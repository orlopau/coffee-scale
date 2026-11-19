---
title: Erste Schritte
weight: 1
---

## Der Drehknopf

Du bedienst die Waage mit einem einzigen Knopf. Er kann drei Dinge:

- **Drehen**, um durch Listen zu blättern und Werte zu ändern. Beim Wiegen tariert Drehen die Waage.
- **Klicken**, um etwas auszuwählen, zu starten oder zu stoppen.
- **Lange drücken** (etwa eine Sekunde halten), um zurückzugehen oder die Liste der Modi zu öffnen.

## Laden

Die Waage lädt über USB-C. Den Akkustand siehst du in der Liste der Modi (lange drücken): links unten ein Symbol, rechts unten die Spannung des Akkus. Beim Laden zeigt das Symbol einen Stecker.

{{< screen name="mode_switcher_0" caption="Die Liste der Modi, links unten der Akku" >}}

## Einschalten

Schalte die Waage mit **leerer Plattform** ein: Sie setzt ihren Nullpunkt gleich beim Start. Danach öffnet sie den Waagen-Modus.

Vorher stellt sie vielleicht eine Frage:

- Beim allerersten Start fragt sie, ob du das WLAN einrichten willst, siehe unten.
- Wenn sie eine neue Firmware gefunden hat, fragt sie, ob sie sie installieren soll, siehe [Updates](../updates/).

## WLAN einrichten

Die Waage sucht über das WLAN nach Updates und spricht darüber mit einer [Mahlkönig-Mühle](../grinder/). Du richtest es vom Handy aus ein, ohne App.

{{% steps %}}

### Ja sagen

Beim ersten Start fragt die Waage **WLAN einrichten?**. Dreh auf **Ja** und klick. Später findest du dasselbe in den [Einstellungen](../settings/) unter **WLAN**.

{{< screen name="wifi_ask_yes" caption="WLAN einrichten?" >}}

### QR-Code scannen

Die Waage öffnet ein eigenes WLAN und zeigt einen QR-Code. Halte die Kamera deines Handys darauf und verbinde dich. Es geht auch von Hand: Der Name des Netzwerks steht auf dem Display, das Passwort ist `coffee-scale`.

{{< screen name="wifi_join" caption="Das eigene WLAN der Waage mit seinem QR-Code" >}}

### Dein WLAN wählen

Auf dem Handy öffnet sich die Einrichtungsseite von selbst. Falls nicht, öffne den Browser und geh auf `http://192.168.4.1`. Wähle dein Netzwerk, gib das Passwort ein und tippe auf **Verbinden**.

### Auf „Verbunden.“ warten

Die Waage zeigt **Verbinde...** und dann **Verbunden.**. Die Seite auf dem Handy sagt es auch, du kannst sie schließen.

{{< screen name="wifi_connected" caption="Verbunden." >}}

{{% /steps %}}

War das Passwort falsch oder wurde das Netzwerk nicht gefunden, sagt die Waage das, und auf der Einrichtungsseite kannst du es nochmal versuchen. Ein Klick auf der Waage beendet die Einrichtung, ohne etwas zu speichern. Mehr unter [Probleme](../problems/#wlan).

## Modus wechseln

**Lange drücken** öffnet die Liste der Modi, **drehen** wählt einen aus, **klicken** öffnet ihn:

| Modus | Wofür |
|---|---|
| **Waage** | Wiegen, mit Timer. Siehe [Wiegen](../weighing/). |
| **Rezepte** | Filterkaffee und Aeropress Schritt für Schritt. Siehe [Rezepte](../recipes/). |
| **Espresso** | Shots mit Zielgewicht. Siehe [Espresso](../espresso/). |
| **Kalibrierung** | Der Waage beibringen, was 100 g sind. Siehe [Kalibrieren](../calibration/). |
| **Einstellungen** | Siehe [Einstellungen](../settings/). |
