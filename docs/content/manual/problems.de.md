---
title: Probleme
weight: 9
---

## Das Gewicht stimmt nicht

- Schalte die Waage mit leerer Plattform ein. Sie setzt ihren Nullpunkt beim Start.
- Tariere sie: Dreh den Knopf ein wenig.
- Stimmt es dann noch nicht, [kalibriere](../calibration/) sie mit einem bekannten Gewicht.

## Das Gewicht springt

Stell die Waage auf eine feste, ebene Fläche ohne Erschütterungen. Springt es dann noch, erhöhe **Gewichts-Mittelung Schwellwert** in den [Einstellungen](../settings/).

## Sie tariert, obwohl ich das nicht will

Das kann Tara durch Anheben oder Auto-Tare sein. Wähle in den [Einstellungen](../settings/) eine festere Stufe für **Tara durch Anheben** oder schalte es aus, und prüfe deine **Auto-Tare Gewichte**.

## Ein Shot wird nicht erkannt

- Ist die **Shot-Erkennung** in den [Einstellungen](../settings/) an? Zu Beginn ist sie aus.
- Tariere die Tasse vor dem Shot. Die Waage achtet nur nach dem Tarieren auf einen Shot.
- Ein Stoß beim Start des Shots kann ihn verdecken. Klick dann, um den Timer von Hand zu starten.

## Kalibrierung nicht lesbar

{{< screen name="error_settings_nvs" caption="Kalibrierung nicht lesbar" >}}

Beim Start konnte die Waage ihre Kalibrierung und Einstellungen nicht aus ihrem Speicher lesen. Sie wiegt, aber nicht richtig. Klick, um weiterzumachen, dann [kalibriere](../calibration/) sie und prüfe die [Einstellungen](../settings/). Erscheint das bei jedem Start, ist vielleicht der Speicher der Waage defekt.

## WLAN

| Die Waage sagt | Was tun |
|---|---|
| **Falsches Passwort. Bitte nochmal versuchen.** | Gib das Passwort auf der Einrichtungsseite nochmal ein; Groß- und Kleinschreibung zählen. |
| **Netzwerk nicht gefunden. Bitte nochmal versuchen.** | Ist das WLAN an und in Reichweite? Die Waage kann nur 2,4-GHz-WLAN. |
| **Verbindung fehlgeschlagen. Bitte nochmal versuchen.** | Versuch es nochmal; stell die Waage näher an den Router. |
| **WLAN startet nicht.** | Schalte die Waage aus und wieder ein. |

{{< screen name="wifi_wrong_password" caption="Falsches Passwort. Bitte nochmal versuchen." >}}

## Updates

| Die Waage sagt | Was tun |
|---|---|
| **Kein Update verfügbar.** | Auf der Waage läuft schon die neueste Firmware. |
| **Fehlgeschlagen.** | Prüfe, ob das WLAN funktioniert, und versuch es nochmal; bei einem automatischen Update fragt die Waage beim nächsten Start wieder. |
| **Update gescheitert. Vorherige Version wiederhergestellt.** | Die neue Firmware startete nicht richtig, deshalb ist die Waage zur vorherigen zurückgekehrt. Nichts zu tun; diese Version wird nicht mehr angeboten. |
