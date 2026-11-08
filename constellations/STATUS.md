# Stand

Stand 2026-11-07, Version 0.1.1: **Constellations** (`saver_constellations`).

## Was er zeigt

- **26 echte Sternbilder** im Rundflug, jedes nah am vorigen: Großer Bär, Kleiner Bär, Drache, Herkules, Nördliche Krone, Bärenhüter, Jungfrau, Skorpion, Schütze, Adler, Delfin, Leier, Schwan, Kepheus, Kassiopeia, Andromeda, Pegasus, Widder, Perseus, Fuhrmann, Stier, Orion, Großer Hund, Kleiner Hund, Zwillinge, Löwe. Der Start ist jedes Mal ein anderes.
- **Ablauf je Sternbild:**
  1. Der Blick gleitet auf einem Großkreis hin, 5 bis 11 s.
  2. Die Figur zeichnet sich in 4 s Linie für Linie ein.
  3. Sie bleibt 11 s stehen, der Blick rückt dabei leicht näher.
  4. Beim Weitergleiten verblasst sie in 2,5 s.
- **Beschriftung:** Der deutsche Name steht unten, darunter der lateinische. Die hellen Sterne der Figur werden benannt (Beteigeuze, Rigel, Wega, Plejaden …). Umlaute sind als ae/oe/ue geschrieben, weil die Schriften nur ASCII kennen.
- **Die anderen Figuren** stehen schwach im Hintergrund, wie in einem Sternatlas.
- **Sterne:**
  - Die Sterne der Figuren und 22 helle Sterne daneben haben ihre echten Positionen (J2000), Helligkeiten und Farben: Beteigeuze und Antares rötlich, Rigel und Wega bläulich.
  - Helle Sterne sind kleine Kreuze, schwache Punkte.
  - Dazu kommen 1500 schwache Sterne. Zwei Drittel davon liegen entlang der Milchstraße, an ihrem echten Ort und zur Mitte hin dichter. Ihre einzelnen Positionen sind erfunden.
- **Alle Sterne funkeln sanft:** Jeder wird in seinem eigenen Takt (1,7 bis 5 s) gleitend heller und dunkler.
- **Sternschnuppen:** Etwa alle 30 s eine, die gleitend auf- und abblendet.
- **Musik, alles gleitend:** Bei lauter Musik geht der Rundflug etwas schneller, die Sterne funkeln lebhafter und Sternschnuppen kommen öfter. Bei einem neuen Track steht 7 s lang sein Titel oben.

## Geprüft

- 0.1.1: Die Sternbilder haben auf dem 4:3-Monitor ihre echten Proportionen. Vorher waren sie um ein Drittel zu breit.

- PC- und Pi-Build ohne Warnungen. `build/saver_constellations-0.1.0.vps` wird erzeugt.
- Am PC Bilder angesehen, mit vierfachem Tempo über zwei Runden: Die Formen stimmen, zum Beispiel Orion, das Pegasus-Quadrat, das W der Kassiopeia, das Kreuz des Schwans und die Teekanne des Schützen. Auch die Lage stimmt, Norden oben und Osten links wie am Himmel.
- Am PC 40 Bilder pro Sekunde mit 150 bis 300 Linien.

## Offen

- Am 2026-11-08 installiert (0.1.1): 40 Bilder pro Sekunde, 70 bis 230 Linien, keine verworfenen Linien. Am echten Monitor noch nicht beurteilt.
- Am echten Monitor noch nicht beurteilt: ob die schwachen Sterne und die schwachen Figuren sichtbar genug sind.
- Ein paar Figurenlinien sind vereinfacht (Großer Bär, Drache, Jungfrau), so wie viele Sternkarten sie zeichnen.
