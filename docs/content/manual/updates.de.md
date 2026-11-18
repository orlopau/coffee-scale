---
title: Updates
weight: 7
---

Neue Firmware bringt neue Funktionen und Korrekturen. Die Waage findet sie von selbst, sobald das [WLAN eingerichtet](../getting-started/#wlan-einrichten) ist.

## Automatische Updates

Alle paar Starts sucht die Waage im Hintergrund nach neuer Firmware, während du wiegst. Davon merkst du nichts. Findet sie eine, fragt sie beim nächsten Einschalten:

{{< screen name="updater_ask_yes" caption="Update auf v10.10.10?" >}}

- **Ja** installiert das Update und startet die Waage neu. Das Display zeigt den Fortschritt; lass die Waage währenddessen an.
- **Später** überspringt es für jetzt. Die Waage fragt beim nächsten Start wieder.

{{< screen name="updater_progress" caption="Lade: 42,17 %" >}}

## Update von Hand

Du kannst die neueste Firmware auch sofort installieren:

{{% steps %}}

### Beim Einschalten den Knopf halten

Halte den Knopf gedrückt, während du die Waage einschaltest, und lass dann los.

### Auf das WLAN warten

Die Waage verbindet sich mit deinem WLAN und zeigt **WiFi verbunden.**. Klappt das nicht, öffnet sie die [WLAN-Einrichtung](../getting-started/#wlan-einrichten).

### Sprache wählen

**Dreh** auf die Sprache, in der die Waage sprechen soll, und **klick**. Die Waage lädt die neueste Firmware in dieser Sprache, installiert sie und startet neu.

{{< screen name="updater_language_0" caption="Choose Language" >}}

{{% /steps %}}

Läuft auf der Waage schon die neueste Firmware, zeigt sie **Kein Update verfügbar.**.

## Wenn ein Update schiefgeht

Startet eine neue Firmware nicht richtig, kehrt die Waage von selbst zur vorherigen zurück. Beim nächsten Start sagt sie dir das und bietet diese Version nicht mehr an. Klick, um weiterzumachen.

{{< screen name="updater_rolled_back" caption="Update gescheitert. Vorherige Version wiederhergestellt." >}}
