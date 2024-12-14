#pragma once

#if defined(LANG_DE)
/////////////////// GERMAN ///////////////////
#define R_AEROPRESS_JH_NAME "Aeropress J.H."
#define R_AEROPRESS_JH_DESC "\"The Ultimate Aeropress Technique\" von James Hoffmann."
#define R_AEROPRESS_JH_0 "Wasser hinzufügen, Kaffeemehl sollte nass sein. Dann Plunger 1cm einführen."
#define R_AEROPRESS_JH_1 "Sanft schwenken um gleichmäßiges Bett zu erzeugen."
#define R_AEROPRESS_JH_2 "Plunger bis zum Boden drücken."

#define R_FRENCH_PRESS_JH_NAME "French Press J.H."
#define R_FRENCH_PRESS_JH_DESC "\"The Ultimate French Press Technique\" von James Hoffmann."
#define R_FRENCH_PRESS_JH_0 "Wasser in French Press gießen."
#define R_FRENCH_PRESS_JH_1 "Warten bis sich der Kaffee absetzt."
#define R_FRENCH_PRESS_JH_2 "Mit zwei Löffeln den Kaffee umrühren und die Kruste entfernen."
#define R_FRENCH_PRESS_JH_3 "Plunger in die French Press einfügen, aber oben auf dem Wasser lassen."

#define R_V60_KASUYA_NAME "V60 Kasuya"
#define R_V60_KASUYA_DESC "V60 5-Aufgüsse Rezept von Tetsu Kasuya."
#define R_V60_KASUYA_0 "Das Verhältnis der ersten beiden Aufgüsse bestimmt die Süße."
#define R_V60_KASUYA_1 "Das Verhältnis der ersten beiden Aufgüsse bestimmt die Süße."
#define R_V60_KASUYA_2 "Die letzten 3 Aufgüsse bestimmen die Stärke des Kaffees."
#define R_V60_KASUYA_3 "Die letzten 3 Aufgüsse bestimmen die Stärke des Kaffees."
#define R_V60_KASUYA_4 "Die letzten 3 Aufgüsse bestimmen die Stärke des Kaffees."

#define R_V60_RAO_NAME "V60 Scott Rao."
#define R_V60_RAO_DESC "V60 recipe by Scott Rao."
#define R_V60_RAO_0 "Kaffee Blooming. Nach dem gießen leicht mit einem Löffel umrühren."
#define R_V60_RAO_1 "Wasserkocher auf einer Höhe halten und Wasser gleichmäßig gießen."
#define R_V60_RAO_2 "Leicht mit einem Löffel rühren."
#define R_V60_RAO_3 "Die V60 leicht kreisen, verhindert das Kleben an der Seite."
#define R_V60_RAO_4 "Warten bis der Kaffee durchgelaufen ist."

#define R_V60_HARIO_NAME "V60 Hario Original"
#define R_V60_HARIO_DESC "Original V60 Rezept von Hario."
#define R_V60_HARIO_0 "Kaffee Blooming. In einer kreisförmigen Bewegung gießen."
#define R_V60_HARIO_1 "Den Rest des Wassers in einer spiralförmigen Bewegung gießen."
#define R_V60_HARIO_2 "Warten bis der Kaffee durchgelaufen ist."

#define DISPLAY_RECIPE_SWITCHER_TITLE "Rezept wählen."
#define DISPLAY_CONFIG_WEIGHT_COFFEE "Kaffee:"
#define DISPLAY_CONFIG_WEIGHT_WATER "Wasser:"
#define DISPLAY_CONFIG_RATIO "Kaffee:Wasser"
#define DISPLAY_INSERT_COFFEE "Kaffee hinzufügen."

#define UPDATER_PROGRESS "Lade: %.2f%%"
#define UPDATER_UPDATING "Aktualisiere..."
#define UPDATER_FAILED "Fehlgeschlagen."
#define UPDATER_SUCCESS "Erfolgreich."
#define UPDATER_NO_UPDATE "Kein Update verfügbar."
#define UPDATER_WIFI_CONNECTED "WiFi verbunden."
#define UPDATER_WIFI_CONNECT_MANUAL "WiFi einrichten:\nMit Netzwerk\n%s\verbinden und\n\"Configure new AP\"\nwählen."

#define MODE_NAME_CALIBRATE "Kalibrierung"
#define MODE_NAME_RECIPES "Rezepte"
#define MODE_NAME_SCALE "Waage"
#define MODE_NAME_ESPRESSO "Espresso"
#define MODE_NAME_SETTINGS "Einstellungen"

#define SETTINGS_TARE_0 "Auto-Tare Gewicht 1"
#define SETTINGS_TARE_1 "Auto-Tare Gewicht 2"
#define SETTINGS_TARE_2 "Auto-Tare Gewicht 3"
#define SETTINGS_TARE_3 "Auto-Tare Gewicht 4"
#define SETTINGS_TARE_4 "Auto-Tare Gewicht 5"
//////////////////////////////////////////////
#else
/////////////////// ENGLISH ///////////////////
#define R_AEROPRESS_JH_NAME "Aeropress J.H."
#define R_AEROPRESS_JH_DESC "\"The Ultimate Aeropress Technique\" by James Hoffmann."
#define R_AEROPRESS_JH_0 "Add water wetting all the grounds, then insert the plunger 1cm into the brewer."
#define R_AEROPRESS_JH_1 "Gently swirl the brewer."
#define R_AEROPRESS_JH_2 "Press the plunger all the way in."

#define R_FRENCH_PRESS_JH_NAME "French Press J.H."
#define R_FRENCH_PRESS_JH_DESC "\"The Ultimate French Press Technique\" by James Hoffmann."
#define R_FRENCH_PRESS_JH_0 "Pour water into the French Press."
#define R_FRENCH_PRESS_JH_1 "Wait until the coffee has settled."
#define R_FRENCH_PRESS_JH_2 "Stir the coffee with two spoons and remove the crust."
#define R_FRENCH_PRESS_JH_3 "Insert the plunger into the French Press, but leave it on top of the water."

#define R_V60_KASUYA_NAME "V60 Kasuya"
#define R_V60_KASUYA_DESC "V60 5-Pour recipe by Tetsu Kasuya."
#define R_V60_KASUYA_0 "The ratio of the first two pours determines the sweetness."
#define R_V60_KASUYA_1 "The ratio of the first two pours determines the sweetness."
#define R_V60_KASUYA_2 "The last 3 pours determine the strength of the coffee."
#define R_V60_KASUYA_3 "The last 3 pours determine the strength of the coffee."
#define R_V60_KASUYA_4 "The last 3 pours determine the strength of the coffee."

#define R_V60_RAO_NAME "V60 Scott Rao."
#define R_V60_RAO_DESC "V60 recipe by Scott Rao."
#define R_V60_RAO_0 "Bloom the coffee. After pouring, gently stir with a spoon, wetting all grounds."
#define R_V60_RAO_1 "Main pour. Keep kettle at one height, evenly pour the water."
#define R_V60_RAO_2 "After pouring, gently stir."
#define R_V60_RAO_3 "Grab the brewer and spin slightly, preventing grounds from sticking to the walls."
#define R_V60_RAO_4 "Wait until the coffee has drained."

#define R_V60_HARIO_NAME "V60 Hario Original"
#define R_V60_HARIO_DESC "Original V60 recipe by Hario."
#define R_V60_HARIO_0 "Bloom the coffee, pouring water in a spiralling motion."
#define R_V60_HARIO_1 "Pour the rest of the water in a spiralling motion. Click when finished."
#define R_V60_HARIO_2 "Wait until the coffee has drained."

#define DISPLAY_RECIPE_SWITCHER_TITLE "Select a recipe."
#define DISPLAY_CONFIG_WEIGHT_COFFEE "Coffee:"
#define DISPLAY_CONFIG_WEIGHT_WATER "Water:"
#define DISPLAY_CONFIG_RATIO "Brew ratio:"
#define DISPLAY_INSERT_COFFEE "Insert coffee."

#define UPDATER_PROGRESS "Updating: %.2f%%"
#define UPDATER_UPDATING "Updating..."
#define UPDATER_FAILED "Update failed."
#define UPDATER_SUCCESS "Update successful."
#define UPDATER_NO_UPDATE "No update available."
#define UPDATER_WIFI_CONNECTED "WiFi connected."
#define UPDATER_WIFI_CONNECT_MANUAL "Setup WiFi by\nconnecting to the\nnetwork:\n%s\nand clicking\n\"Configure new AP\"."

#define MODE_NAME_CALIBRATE "Calibration"
#define MODE_NAME_RECIPES "Recipes"
#define MODE_NAME_SCALE "Scale"
#define MODE_NAME_ESPRESSO "Espresso"
#define MODE_NAME_SETTINGS "Settings"

#define SETTINGS_TARE_0 "Auto-Tare Weight 1"
#define SETTINGS_TARE_1 "Auto-Tare Weight 2"
#define SETTINGS_TARE_2 "Auto-Tare Weight 3"
#define SETTINGS_TARE_3 "Auto-Tare Weight 4"
#define SETTINGS_TARE_4 "Auto-Tare Weight 5"
//////////////////////////////////////////////
#endif