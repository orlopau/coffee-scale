---
title: Espresso
weight: 3
---

Der Espresso-Bildschirm stoppt die Zeit eines Shots und zeigt, wie nah er am Zielgewicht ist.

{{< screen name="espresso_brewing" caption="Ein laufender Shot: 4,0 s vorbei, noch etwa 3,2 s, 5,0 g/s, 20,0 g von 36,0 g" >}}

- **Oben:** die Zeit seit dem Start des Shots. Sobald die Waage schätzen kann, wann der Shot das Ziel erreicht, steht die verbleibende Zeit links daneben, z. B. `-3.2s|4.0s`.
- **Mitte:** der Durchfluss in Gramm pro Sekunde, solange der Shot läuft.
- **Unten:** das Gewicht in der Tasse und das Zielgewicht, mit einem Balken, der sich füllt.

## Ein Shot von Hand

{{% steps %}}

### Ziel einstellen

Öffne **Espresso** in der Liste der Modi. **Dreh** den Knopf, um das Zielgewicht einzustellen, in Schritten von 0,1 g, von 1 g bis 100 g. Es beginnt bei 36,0 g.

{{< screen name="espresso_idle" caption="Bereit, mit einem Ziel von 36,0 g" >}}

### Starten

Stell die Tasse auf die Waage und **klick**, wenn du die Pumpe startest. Die Waage tariert und startet den Timer.

### Stoppen

Der Timer stoppt von selbst, wenn die Tasse das Zielgewicht erreicht, oder wenn du noch einmal **klickst**.

{{% /steps %}}

## Nach dem Shot

Wenn der Shot endet, zeigt die Waage seine ganze Kurve: das Gewicht über die Zeit, die Menge in der Tasse auf einer gestrichelten Linie in ihrer Höhe und rechts unten die Zeit des Shots. Der Shot davor ist gepunktet dahinter gezeichnet, so kannst du vergleichen.

{{< screen name="espresso_shot_with_last" caption="36,5 g in 28,0 s, der Shot davor gepunktet dahinter" >}}

**Klick**, um zum Espresso-Bildschirm zurückzugehen; der nächste Klick startet einen neuen Shot. Tarieren, z. B. durch Anheben der Plattform für die nächste Tasse, geht auch zurück.

{{< callout type="info" >}}
Mit einer Mahlkönig E64 WS öffnet ein Druck auf den Knopf der Mühle diesen Bildschirm mit dem Gewicht deines Rezepts als Ziel, und die Mühle stellt sich nach dem Shot selbst ein. Siehe [Mahlkönig-Mühle](../grinder/).
{{< /callout >}}

## Shot-Erkennung

Mit eingeschalteter Shot-Erkennung musst du nicht klicken: Die Waage bemerkt den Shot von selbst.

{{% steps %}}

### Tasse tarieren

Stell die Tasse auf die Waage und tariere sie, im Waagen-Modus oder auf dem Espresso-Bildschirm.

### Shot ziehen

Starte die Pumpe. Wenn die ersten Tropfen ein paar Sekunden gleichmäßig geflossen sind, wechselt die Waage zum Espresso-Bildschirm (wenn sie im Waagen-Modus war) und startet den Timer, gezählt ab dem Start der Pumpe.

### Pumpe stoppen

Der Timer stoppt in dem Moment, in dem die Pumpe aufhört, nicht erst, wenn die letzten Tropfen fallen. Ein Klick stoppt ihn auch.

{{% /steps %}}

{{< callout type="info" >}}
Die Shot-Erkennung ist zu Beginn **aus**. Schalte sie in den [Einstellungen](../settings/) unter **Shot-Erkennung** ein.
{{< /callout >}}

Gut zu wissen:

- Die Waage achtet nur nach dem Tarieren auf einen Shot. Tropfen nach dem Shot oder eine abgestellte Tasse starten keinen.
- Bohnen in eine tarierte Tasse zu mahlen oder abzuwiegen startet keinen Shot: Bohnen fallen ungleichmäßig, ein Shot fließt gleichmäßig.
- Stößt du beim Start des Shots an die Waage, bemerkt sie ihn vielleicht nicht. Dann klick, um den Timer von Hand zu starten.
- Rezepte, die Kalibrierung und die Einstellungen werden nie unterbrochen.
- Das Zielgewicht stoppt einen erkannten Shot nicht; nur das Ende der Pumpe oder ein Klick.

### Shot-Vorlauf

Die Waage sieht die Pumpe nicht, nur den ersten Tropfen, und der kommt ein paar Sekunden nach dem Pumpenstart. Deshalb startet sie den Timer um so viele Sekunden früher. **Shot-Vorlauf** in den Einstellungen legt diese Sekunden fest, Standard ist 7,0. Wirken deine Shot-Zeiten im Vergleich zur Maschine zu lang oder zu kurz, ändere ihn: Miss an deiner Maschine die Zeit vom Pumpenstart bis zum ersten Tropfen und nimm diese.
