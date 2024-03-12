# Dateiverwaltung

Der Wecker benutzt eine Micro-SD-Karte, um dort gespeicherte Musik zum Wecken zu verwenden. Er stellt außerdem grundlegende Dateiverwaltung zur Verfügung, allerdings muss die zu verwendende Musik vorher mit einem anderen Gerät auf der SD-Karte abgelegt werden.

- auto-gen TOC;
  {:toc}

## Kartenformat

Der Wecker unterstützt nur Karten, die ein FAT-Dateisystem (FAT16, FAT32, exFAT) haben und entsprechend dem SD-Standard formatiert sind. Die Formatierung nach diesen Kriterien wird z.B. vom offiziellen SD-Karten-Formatierungstool der SD Association durchgeführt, welches [hier](https://www.sdcard.org/downloads/formatter/) kostenlos heruntergeladen werden kann.

Zum Einlegen oder Entnehmen einer SD-Karte sollte der Wecker nicht eingeschaltet sein, da dies die Daten auf der SD-Karte beschädigen kann.

## Dateiansicht

Verschiedene Menüs des Weckers bieten eine Dateiansicht, die die Dateien eines Verzeichnisses auflistet. Das sind z.B. das Lösch- und Verschiebemenü sowie die Menüs zur Auswahl einer Musikdatei für einen Wecker. Die Menüs besitzen immer denselben Aufbau: In einer Liste, wie auch in anderen Listenmenüs, werden die Dateien des aktuellen Ordners angezeigt, zwischen denen mit _Oben_ und _Unten_ navigiert werden kann. Die ersten beiden Einträge dieser Liste sind besonders.

Der erste Eintrag hat als Namen den vollständigen Pfad des aktuellen Verzeichnisses. Je nach Art des Menüs kann dieser Eintrag so wie andere Dateien ebenfalls angewählt werden, um das Verzeichnis (anstatt einer einzelnen Datei) anzuwählen. Beispielsweise kann ein Verzeichnis im Löschmenü angewählt werden, um das gesamte Verzeichnis zu löschen. Besteht keine Anwahlmöglichkeit für ein Verzeichnis (d.h. es können nur Dateien oder überhaupt nichts angewählt werden), so kann _Rechts_ dennoch betätigt werden, um die Verzeichnisansicht zu aktualisieren. (In der Regel sorgt dies nicht für eine sichtbare Änderung der Anzeige.)

Der zweite Eintrag hat als Namen "..". Dieser Eintrag beschreibt, so wie auch in Klammern angegeben ist, das Elternverzeichnis des aktuellen Verzeichnisses, also das nächste übergeordnete Verzeichnis. Die Anwahl dieses Eintrags öffnet das übergeordnete Verzeichnis in der Dateiansicht. (Dieser Eintrag hat keine Auswirkungen, sollte bereits das Basisverzeichnis der SD-Karte geöffnet sein.) Dazu ist wichtig zu bemerken, dass _Links_ absichtlich **nicht** das Elternverzeichnis öffnet, sondern das Dateimenü schließt und zum vorherigen Menü zurückkehrt. Dies erleichtert das Verlassen der Dateiansicht, wenn gerade ein tief verschachteltes Dateimenü geöffnet ist. Da der ".."-Eintrag der zweite Eintrag der Liste ist, ist es recht einfach, schnell einige Verzeichnisse nach oben zu navigieren.

Das Anwählen von Unterverzeichnissen, die als normale Einträge geführt werden, öffnet dieses Unterverzeichnis in der Dateiansicht. Soll dieses Verzeichnis selbst angewählt werden, kann in der jetzt sichtbaren Liste entsprechend der erste Eintrag gewählt werden.

Die Sortierung der gewöhnlichen Einträge hat aktuell technische Hintergründe und entspricht vermutlich dem Erstellzeitpunkt der Dateien bzw. Verzeichnisse.

## Dateiverwaltungsmenü

Das Menü "Dateiverwaltung", welches sich im Hauptmenü findet, ermöglicht generelle Verwaltung der Dateien auf der SD-Karte. Die folgenden Aktionen sind möglich:
- Dateiansicht: Ansicht der Dateien auf der SD-Karte. Aktuell bietet dies lediglich die Möglichkeit, durch die Verzeichnisse zu navigieren und die Dateien der verschiedenen Verzeichnisse anzuzeigen. Die Anwahl von Dateien oder Verzeichnissen hat keine Funktion.
- Verschieben: Verschieben von Dateien in andere Verzeichnisse. Nach der Anwahl einer Datei kann ein anderes Verzeichnis ausgewählt werden. Nach der Bestätigung der Verschiebeaktion wird die Datei ans Ziel verschoben.
- Löschen: Löschen von Verzeichnissen oder Dateien. Nach der Anwahl einer Datei oder eines Verzeichnisses und der Bestätigung wird das entsprechende Element dauerhaft gelöscht. Der Wecker verfügt nicht über einen Papierkorb!

Der Wecker verfügt bewusst nicht über eine Umbenennungsfunktion, da Texteingabe mit vier Knöpfen unmöglich gut funktionieren kann. Für komplexere Dateiverwaltungsaufgaben sollte die SD-Karte mit einem Computer verbunden werden.
