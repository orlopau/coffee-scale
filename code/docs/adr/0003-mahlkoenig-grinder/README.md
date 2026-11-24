# 3. Play the Xenia for a Mahlkönig E64 WS, with the time to the target weight

Status: proposed, 2026-11-18. Accepted once confirmed with the real E64 WS.

## Context

The Mahlkönig E64 WS dials itself in with Grind-by-Sync (GbS): after each shot it compares the shot time with the recipe's Brew Time and moves its grind setting finer or coarser. For that it needs an espresso machine to report the shot, Mahlkönig's own Xenia. Without one, GbS does nothing.

The scale already measures what the grinder needs: with shot detection (ADR 2) it times a shot from the pump start, and it weighs the cup. So the scale should report the shots to the grinder, and the grinder should dial itself in as with a Xenia.

There is nothing else at the machine: no Pi, no server, no laptop. Whatever talks to the grinder runs on the scale, and setting it up must work from the grinder and the scale alone, without a computer or router settings.

Before designing it, we read how the grinder and a Xenia talk and what a Xenia measures, and let that decide. The sources are listed below. gbs-anywhere is only the reference for the protocol's facts, which could as well have been found by listening to a Xenia; the scale's code is our own.

## The sources

| Source | What it tells |
| --- | --- |
| [gbs-anywhere](https://github.com/Skamba/gbs-anywhere) | A program that plays the Xenia on a computer, for the E64 WS and the E65/E80 GbS. Its protocol types (`src/protocol/`) and the E64 WS server (`src/server/grinder_api.rs`): the requests, the keys, the values and the grinder's rules |
| [Brew-by-Sync Onboarding Guide](https://downloads.mahlkoenig.de/Service/Mahlkoenig_Xenia_Brew_by_Sync_Onboarding_Guide.pdf) | How a grinder and a Xenia are connected: the grinder's scan |
| [Xenia manual](https://downloads.mahlkoenig.de/Service/Mahlkoenig_Xenia_Brew_by_Sync_Instruction_Manual_EN.pdf) | The Xenia's own name on the network (4.2), its connection to the E64 WS (4.3), its scale and how it stops at the brew weight (6.3, 6.7) |
| [E64 WS software manual](https://downloads.mahlkoenig.de/Service/Mahlkoenig_E64_WS_Espresso_Home_Software_Manual.pdf) | The grinder's Connectivity and Machine to Machine menus (2.8) |

Not available: a Xenia or a recording of one talking to the grinder. What the grinder's scan asks for is therefore still unknown, see Consequences.

Added on 2026-11-24: a recording of the real E64 WS talking to `scripts/fake_xenia.py`, which answers like the scale, through one GbS grind and knob press, with the grinder's requests, their headers, bodies and replies. Where it differs from gbs-anywhere, section 2 follows the recording.

## How we got there

### 1. Two families of grinders, two directions

| Grinder | Who asks whom | What it would take on the scale |
| --- | --- | --- |
| E64 WS | The grinder asks the machine about every 0.8 s, plain HTTP on port 80 | A small web server on the scale |
| E65/E80 GbS | The machine asks the grinder, HTTPS on port 30001, after pairing with a code on the grinder | A client with pairing, and a stored pairing ID |

- The E64 WS is the one we have, so only that one.
- A third way, the scale reporting to gbs-anywhere, which then talks to the grinder, needs a computer next to the grinder. Ruled out.

### 2. How the E64 WS talks to its machine

The grinder is the client, the machine the server: port 80, plain HTTP, JSON.

| Request | When | The machine answers |
| --- | --- | --- |
| `GET /api/v2/mako` | every 0.4–1.9 s, median 0.8 s | the machine state, below |
| `GET /api/v2/machine` | twice when it connects | the machine's identity |
| `POST /api/v2/brewratio` | after a grind with a GbS recipe, three times | `{}`. The body holds the recipe's Brew Weight: `{"SYNC_BEVERAGE_WEIGHT":38,"SYNC_FILTER":2}`, g, both plain numbers |
| `POST /api/v2/scripts/execute` | the knob press after a GbS grind, `{"ID":9}`, three times | `{}`, and starts the shot |

What the recording adds to gbs-anywhere:

- **One connection for everything.** The grinder opens one HTTP/1.1 keep-alive connection and sends every request on it, the polls and the POSTs alike. Its headers: `Content-Type: application/json` (also on the GETs), `Connection: Keep-Alive`, `Accept-Encoding: gzip, deflate`, `Accept-Language: en,*`, `User-Agent: Mozilla/5.0`. No `Origin` or `Sec-Fetch-*`.
- **Polls every 0.4–1.9 s,** median 0.8 s, not every 2 s. The longest gaps fall around the POSTs.
- **Every POST three times,** 8.2–8.8 s apart, the same body each time. The knob press was repeated although the grinder had already seen "serving" in the polls after the first, so the repeats don't wait for an answer: they come whatever the machine does. The brew ratio came 21 s before the knob press, after the grind.
- **Nothing else:** no other path, no other port, no mDNS question while it was set to an address under Edit Host.

The state the grinder reads:

| Key | Ready | Shot running | Shot finished |
| --- | --- | --- | --- |
| `MA_STATUS` | 1 (on) | 3 (serving) | 4 (finishing) |
| `MA_EXTRACTION_STATUS` | 0 | 1 | 1, or **2: aborted** |
| `REAL_EXTRACTION_TIME_MS` | last shot's | so far | the shot's time |
| `PU_SENS_FLOW_METER_VOLUME` | last shot's | 0 | the shot's weight (1 g ≈ 1 ml) |
| `BG_SET_TEMP`, `BG_SENS_TEMP` | 93.0, 93.0 | same | same |
| `TANK_LEVEL` | 1 | 1 | 1 |

- It grinds only while the machine is on (or finishing), heated (`BG_SENS_TEMP > BG_SET_TEMP − 2`) and its tank full (`TANK_LEVEL` 1).
- Only a shot the grinder started counts: after its knob press, the machine going to "serving" makes it a shot. A brew the machine starts by itself is a flush.
- It takes the result when it sees "finishing", from one of its polls, so "finishing" has to last longer than one poll.
- It adjusts from the time only. The weight is stored, not used. "Aborted" makes it skip the shot, and so does a time of 10 s or less or over 80 s.

### 3. How the grinder finds its machine

- Mahlkönig's steps: on the grinder, Connectivity → Machine To Machine → Enable Xenia, then Configuration → **Scan**, and pick the Xenia from the list. No fixed IP.
- A Xenia answers as `xenia-coffee.local`, so the scan most likely looks for it over mDNS. What exactly it asks for, the manuals don't say.
- gbs-anywhere announces nothing and isn't found by the scan. Its users type the computer's IP under Configuration → **Edit Host**, which also takes a name. It finds an IP more reliable than a `.local` name.

### 4. What a Xenia's time means

A Xenia has a scale built in. With GbS it takes the Brew Weight from the grinder's recipe and stops its pump just before it, allowing for the drips (Xenia manual 6.3, 6.7). So the time the grinder compares with its Brew Time is **the time until the cup holds the Brew Weight**.

The scale can't stop the pump; the person does. If the scale sent the time to the pump stop, as shot detection measures it, every mistimed stop would teach the grinder something wrong:

| Stop | Time sent | The grinder |
| --- | --- | --- |
| Late, 45 g on a 40 g recipe | about 2.5 s too long at 2 g/s | grinds coarser, though the grind was right |
| Early, 32 g on a 40 g recipe | too short | grinds finer, though the grind was right |
| Target 36 g on the scale, recipe 40 g | too short, every shot | grinds finer and finer |

- So the scale has to measure what a Xenia measures, the time to the recipe's weight, whatever the display shows and whenever the pump stops.
- The target has to be the recipe's Brew Weight, not whatever is set on the scale.
- A shot that never reaches the weight has no such time. Estimating it from the flow would be a guess, and a guess teaches the grinder worse than a skipped shot.

### 5. Wi-Fi

- Today the scale turns Wi-Fi on only while it needs it, for updates and uploads.
- The grinder's knob press arrives as a request to the scale, so the scale has to be reachable before it.
- Wi-Fi only on the espresso screen would mean opening it a few seconds before every knob press. So Wi-Fi stays on, with the radio's power save between the grinder's polls.

## Decision

- **The scale is the Xenia** for an E64 WS: a web server on port 80 answers the grinder as in section 2. E64 WS only, nothing besides the scale.
- **One setting, "Mahlkönig grinder", off by default.** Off, nothing changes. On, the scale stays connected to its Wi-Fi whenever it is on, runs the server and announces itself over mDNS. Turning it on without a saved network opens the Wi-Fi setup first.
- **Found by the grinder's scan,** by announcing what a Xenia announces, once we know what that is. Until then the scale also answers as `xenia-coffee.local` (the Xenia manual) and `xenia.local` (what Xenia owners use), only while the setting is on, in case the scan asks for a name. As the fallback, the user types the scale's name under Edit Host. The settings show the scale's name and IP.
- **The knob press acts where shot detection acts.** In the scale mode it switches to the espresso screen, on the espresso screen it stays there; either way the scale tares (the cup is already on) and tells the grinder the shot is running. Recipes, calibration, the settings and the mode switcher are never interrupted: a knob press there is answered as an aborted shot.
- **The recipe's Brew Weight becomes the target.** At the knob press the espresso screen takes the Brew Weight the grinder sent after the grind. It stays the target like one set by hand; the encoder still changes it.
- **The grinder always gets the time until the cup reaches the target weight.** It starts where the espresso stopwatch starts: at the estimated pump start with shot detection on, at the click with it off. It ends at the first reading at or above the target weight. The display stays as it is, so with shot detection on it still runs to the pump stop. The weight sent is the weight in the cup at the end.
- **Shot detection and the grinder setting are independent.** With shot detection off, the user clicks to start after the knob press, as today, and the shot ends at the target weight or a second click.
- **Anything that isn't a full shot goes to the grinder as aborted,** and the grinder skips it: a shot that ends under the target weight, leaving the espresso screen before the shot ended (opening the mode switcher too), or no shot after the knob press. Drips that reach the target shortly after the pump stop still count, with the time to the pump stop.
- **A repeated knob press is the same press.** The grinder sends each knob press three times, 8.2–8.8 s apart. A knob press within 20 s of the one that started the last shot is ignored, whatever the phase: otherwise a repeat that comes after a quick abort would start a shot nobody pulls and tare the cup. A real second knob press after an abort therefore has to wait those 20 s.
- **A shot without a knob press is never sent.** The grinder sees the machine "on" throughout.
- **The machine is always on, heated and full,** so the grinder never refuses to grind because of it.
- **Built like the detectors:** the Xenia's states and replies in plain C++ without network code, tested natively with a fake clock; on the ESP32 a thin layer with `esp_http_server` and mDNS. The server and the main loop share the one Xenia under a lock; the loop sees the knob press as the Xenia starting a shot.

### The values

| What | Value | From the sources |
| --- | --- | --- |
| Port | 80, plain HTTP | the E64 WS asks there; Edit Host takes no port |
| Grinder's poll | every 0.4–1.9 s, median 0.8 s | the recording |
| "Finishing" held | 6 s, then "on" | longer than one poll, with room for a late one; gbs-anywhere uses the same |
| No shot after the knob press | aborted after 3 minutes; a shot that started runs as long as it takes | gbs-anywhere's default for the time to pull a shot |
| Knob press while "finishing" | ends it and starts the next shot | the grinder has moved on from the last one |
| Repeated knob press | within 20 s of the one that started the shot: ignored | the recording: repeats 8.2–8.8 s and 16.9 s after the press |
| Drips after the pump stop | the target reached within a few seconds after the pump stop still counts | ADR 2: 0.4–1.2 g still reach the cup after the pump stop, within about 1 s |
| Heated | `BG_SET_TEMP` 93.0, `BG_SENS_TEMP` 93.0 | the grinder wants `BG_SENS_TEMP > BG_SET_TEMP − 2` |
| Tank | `TANK_LEVEL` 1 | 1 is full to the grinder |
| Extraction status | 0 ready, 1 serving and finished, 2 aborted | only 2 means something to the grinder; 0 and 1 are gbs-anywhere's best guess |
| Weight sent | `PU_SENS_FLOW_METER_VOLUME` in g as ml | the grinder stores it as a volume, 1 g ≈ 1 ml |
| Identity | `MA_TYPE` 1, `MA_SUBTYPE` 0, a 10-character `MA_SN`, `FW_VERSION` 4.226, `ESP_FW` 3.87, `MA_MAX_AMPERE` 13, `MA_FIX_WATER_SUPPLY` 0, `BLE` 0, `MA_DELAY` −1 | a current Xenia, as gbs-anywhere answers |
| Grinder's range | adjusts from over 10 s up to 80 s | gbs-anywhere |

## Consequences

- With the setting on, a GbS grind, the knob press and the pump are all it takes: the scale times the shot and the grinder adjusts, with nothing typed.
- The grinder learns from the same measurement as with a Xenia, whatever the person does at the pump: a late stop gives the same time, an early one is skipped.
- With shot detection on, the display and the grinder can show different times: the display runs to the pump stop, the grinder gets the time to the target weight.
- The time starts where the espresso stopwatch starts. With shot detection on that is the pump start estimated with the offset, so a wrong offset shifts every time sent by the same amount. The grinder then dials in to a slightly different shot, consistently.
- Wi-Fi on all the time costs battery while the setting is on. How much, we'll see on the scale.
- The setup portal also uses port 80: while it is open, the grinder's server is stopped.
- The grinder's scan (its firmware, read 2026-11-24) browses `_http._tcp` over mDNS and keeps only the entries whose host name contains "Xenia", capital X, then asks them for `/api/v2/machine`. So the scale also announces `_http._tcp` on `Xenia-coffee.local`, besides its own on `coffee-scale.local`. Edit Host with the scale's IP skips the scan.
- What the grinder does while the scale is off is unknown: whether it still grinds GbS recipes or blocks them. If it blocks, the user turns off Enable Xenia, and the setting says so.
- The rules rest on gbs-anywhere's experience with the E64 WS and Mahlkönig's manuals, and on one recording of the grinder with a stand-in on a computer, not on a recording of a Xenia. Why the grinder repeats its POSTs, and whether a real Xenia's answer stops it, is unknown. The first shots on the scale with the real grinder are the test.
- The scale keeps the radio's power save, so a reply can wait for the next DTIM beacon (typically 100–300 ms) while the grinder polls about every 0.8 s. Whether the grinder minds is unknown; the log firmware shows the gaps between its polls.
- The E65/E80 GbS could follow later, as a client with pairing; nothing here stands in its way.
