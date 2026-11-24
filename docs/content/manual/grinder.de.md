---
title: Mahlkönig-Mühle
weight: 4
---

Eine Mahlkönig E64 WS stellt sich mit Grind-by-Sync (GbS) selbst ein: Nach jedem Shot vergleicht sie seine Zeit mit der Brühzeit deines Rezepts und mahlt feiner oder gröber. Dafür braucht sie eine Espressomaschine, die ihr die Zeit sagt, normalerweise Mahlkönigs Xenia. Die Waage kann den Platz der Xenia einnehmen, mit jeder Espressomaschine. Sie braucht nur dein WLAN, kein weiteres Gerät.

{{< screen name="espresso_grinder" caption="Nach dem Knopfdruck: die 40,0 g des Rezepts als Ziel, oben rechts das Zeichen der Mühle" >}}

## Einrichten

{{% steps %}}

### Die Waage ins WLAN bringen

Waage und Mühle müssen im selben Netz sein. Siehe [WLAN einrichten](../getting-started/#wlan-einrichten). Hat die Waage noch kein WLAN, öffnet sie die WLAN-Einrichtung von selbst, wenn du die Einstellung einschaltest und die Einstellungen verlässt.

### Die Einstellung einschalten

Stell in den [Einstellungen](../settings/) **Mahlkönig-Mühle** auf **An**. Während du sie änderst, zeigt die Waage darunter ihren Namen und ihre Adresse. Die brauchst du an der Mühle.

{{< screen name="settings_grinder_on_connected" caption="An, mit dem Namen und der Adresse der Waage" >}}

Die Einstellung gilt, sobald du die Einstellungen verlässt. War sie vorher aus, erscheint die Adresse erst danach: Verlass die Einstellungen und öffne sie noch einmal, um sie zu sehen.

{{< screen name="settings_grinder_on_not_active" caption="Gerade eingeschaltet: aktiv nach dem Verlassen der Einstellungen" >}}

### Die Xenia an der Mühle einschalten

Geh an der Mühle zu **Connectivity** → **Machine To Machine** und schalte **Enable Xenia** ein.

### Die Mühle die Waage finden lassen

Geh zu **Configuration** → **Scan** und wähl die Waage aus der Liste. Steht sie nicht in der Liste, geh zu **Configuration** → **Edit Host** und gib `coffee-scale.local` ein. Klappt auch das nicht, gib die Adresse ein, die die Waage zeigt, z. B. `192.168.1.42`.

### Auf das Zeichen achten

Nach ein paar Sekunden erscheint oben rechts auf dem Display der Waage ein kleines Quadrat. Es zeigt, dass die Mühle mit der Waage spricht. Es bleibt, solange die Mühle die Waage alle paar Sekunden fragt.

{{% /steps %}}

## Einen Shot ziehen

{{% steps %}}

### Mahlen

Mahl wie gewohnt mit einem GbS-Rezept an der Mühle. Die Mühle schickt das Getränkegewicht des Rezepts an die Waage.

### Die Tasse auf die Waage stellen

Die Waage sollte im Waagen-Modus oder auf dem Espresso-Bildschirm sein.

### Den Knopf der Mühle drücken

Die Waage wechselt zum Espresso-Bildschirm, tariert und nimmt das Getränkegewicht des Rezepts als Ziel.

### Den Shot ziehen

Mit eingeschalteter [Shot-Erkennung](../espresso/#shot-erkennung) startest du einfach die Pumpe. Ohne sie **klickst** du wie gewohnt, wenn du die Pumpe startest.

### Die Mühle stellt sich ein

Wenn der Shot endet, schickt die Waage seine Zeit an die Mühle, und die Mühle stellt den Mahlgrad für den nächsten Shot ein.

{{% /steps %}}

## Die Zeit, die die Mühle bekommt

Eine Xenia stoppt ihre Pumpe, wenn die Tasse das Getränkegewicht erreicht. Die Mühle erwartet also **die Zeit, bis die Tasse das Getränkegewicht enthält**. Genau die schickt die Waage, egal wann du die Pumpe stoppst. Stoppst du etwas zu spät, bekommt die Mühle trotzdem die richtige Zeit und mahlt nicht grundlos gröber.

- Die Zeit beginnt, wo der Espresso-Timer beginnt: mit Shot-Erkennung beim Pumpenstart, ohne sie bei deinem Klick.
- Erreicht die Tasse das Getränkegewicht mit den Nachtropfen, innerhalb von 3 Sekunden nach dem Pumpenstopp, zählt der Shot trotzdem, mit der Zeit bis zum Pumpenstopp.
- Das Gewicht in der Tasse wird mitgeschickt. Die Mühle speichert es, stellt sich aber nur nach der Zeit ein.

### Shots, die die Mühle überspringt

Alles, was kein ganzer Shot ist, geht als abgebrochen an die Mühle, und sie stellt sich danach nicht ein:

- der Shot endet, bevor die Tasse das Getränkegewicht erreicht,
- du verlässt den Espresso-Bildschirm, bevor der Shot endet, oder öffnest die Liste der Modi,
- innerhalb von 3 Minuten nach dem Knopfdruck startet kein Shot,
- der Knopf wird gedrückt, während Rezepte, Kalibrierung, die Einstellungen oder die Liste der Modi offen sind, während der Timer im Waagen-Modus läuft oder während ein Shot läuft. Die Waage bleibt, wo sie ist.

Die Mühle selbst ignoriert außerdem Shots von 10 Sekunden oder weniger und von über 80 Sekunden.

## Gut zu wissen

- **Das Ziel bleibt deins.** Du kannst am Knopf das Ziel ändern, das die Mühle gesetzt hat, wie eins, das du selbst eingestellt hast. Die Mühle bekommt trotzdem die Zeit, zu der die Tasse das erste Ziel erreicht hat.
- **Nur Knopfdrücke zählen.** Ein Shot, den du ohne den Knopf der Mühle ziehst, geht nie an die Mühle.
- **Schalte die Waage vor dem Mahlen ein.** Solange sie aus ist, erreicht die Mühle sie nicht.
- **Der Akku leert sich schneller.** Mit der Einstellung bleibt die Waage im WLAN, solange sie an ist. Schalte die Einstellung aus, wenn du die Mühle nicht nutzt. Ist dein WLAN nicht in Reichweite, versucht die Waage es nach ein paar Sekunden erneut, dann immer seltener, höchstens einmal pro Minute.
- **Während der WLAN-Einrichtung** erreicht die Mühle die Waage nicht; danach kommt das Zeichen von selbst zurück. Ist die Waage jetzt in einem anderen Netz, kann sich ihre Adresse geändert haben: Hast du die Adresse an der Mühle eingegeben, gib die neue ein.

## Wenn es nicht klappt

| Was du siehst | Was du tun kannst |
|---|---|
| Die Einstellung zeigt **Nicht verbunden** | Die Waage ist nicht in deinem WLAN. Prüf, ob das Netz in Reichweite ist, oder richte es unter **WLAN** in den Einstellungen neu ein. |
| Kein Zeichen oben rechts | Die Mühle erreicht die Waage nicht. Prüf, ob beide im selben Netz sind und **Enable Xenia** an ist. Versuch unter **Edit Host** die Adresse, die die Waage zeigt, statt `coffee-scale.local`. |
| Der Knopfdruck tut nichts auf der Waage | Die Waage muss im Waagen-Modus oder auf dem Espresso-Bildschirm sein, ohne laufenden Shot. Anderswo überspringt die Mühle den Shot. Innerhalb von 20 Sekunden nach einem Knopfdruck tut ein weiterer nichts: Die Mühle sendet jeden Druck mehrmals, und die Waage nimmt sie als einen. |
| Die Mühle stellt sich nicht ein | Sie überspringt abgebrochene Shots und Shots unter 10 oder über 80 Sekunden. Prüf, ob die Tasse das Getränkegewicht erreicht hat. |
