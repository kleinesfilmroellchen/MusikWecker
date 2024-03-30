# Uhr

Der Uhrbildschirm ist der Startbildschirm des Weckers. Er zeigt die aktuelle Uhrzeit entsprechend dem aktuellen Uhrdesign, das aktuelle Datum (falls aktiviert), sowie rechts oben mehrere Icons, die den Zustand des Weckers beschreiben:

- Funksymbol: Wecker ist mit einem WLAN-Netzwerk verbunden.
- Kreuz anstelle des Funksymbols: Wecker ist nicht mit einem Netzwerk verbunden.
- Uhr mit Funkwellen: Wecker hat soeben seine Uhrzeit synchronisiert. Dies geschieht für gewöhnlich jede Minute, und das Symbol wird einige Sekunden angezeigt.
- Wecker: Mindestens ein Wecker ist aktiv.
- Lautsprecher: Ton wird wiedergegeben.

## Uhrdesign

Das Uhrdesign-Menü (erreichbar im Hauptmenü) erlaubt die Auswahl des Uhrdesigns des Weckers. Wird die Auswahl in diesem Menü mehr als fünf Sekunden nicht bewegt, zeigt die rechte Seite des Bildschirms eine Vorschau des aktuell angewählten Designs. Durch _Rechts_ kann dieses Design ausgewählt werden.

Die folgenden Designs sind aktuell verfügbar:

- Digital: Einfache, große Digitaluhr ohne Sekundenanzeige.
- Digital (Sekunden): Identisch zur normalen Digitaluhr, allerdings mit Sekundenanzeige.
- Analog (minimalistisch): Analoguhr mit Stundenstrichen ohne Beschriftungen.
- Rotierende Segmente: Drei rotierende Segmente, die (von innen nach außen) Stunden, Minuten und Sekunden anzeigen. Die Größe des Segments entspricht dem Abstand eines analogen Zeigers von der Zwölf-Uhr-Position.
- Binär: Binäruhr, die von unten nach oben Stunden, Minuten und Sekunden als Binärzahl anzeigt.
- Binär (Tagsekunden): Binäruhr, die die Sekunden des Tages anzeigt.

## Uhrzeit und Zeitsynchronisation

Der Wecker ruft die aktuelle Uhrzeit über das Internet mithilfe des Network Time Protocols (NTP) ab. Da der Wecker die Uhrzeit nicht persistent speichert, ist er auf die Netzwerkzeitsynchronisation angewiesen, und zeigt beispielsweise nach dem Hochfahren bis zur ersten Netzwerkverbindung keine sinnvolle Uhrzeit an.

Zur Synchronisation muss auf einen NTP-Server zugegriffen werden, der im Idealfall jederzeit verfügbar sein sollte, damit eine Resynchronisierung zu beliebigen Zeitpunkten möglich ist. Aktuell wird immer der `europe.pool.ntp.org`-Server ausgewählt, der einen zufälligen europäischen NTP-Poolserver abruft. Der NTP-Pool ist allgemein zu empfehlen, da er auf viele tausende Server zurückgreifen kann und damit kaum störungsanfällig ist. In einer zukünftigen Firmwareversion werden diese NTP-Server als Auswahl zur Verfügung stehen:

- `pool.ntp.org`, `0.pool.ntp.org`, `1.pool.ntp.org`, `2.pool.ntp.org`, `3.pool.ntp.org`: Die globalen NTP-Poolserver. Die Server mit vorangestellter Ziffer sollten nur verwendet werden, falls die anderen nicht erreichbar sind.
- `africa.pool.ntp.org`, `asia.pool.ntp.org`, `europe.pool.ntp.org`, `north-america.pool.ntp.org`, `oceania.pool.ntp.org`, `south-america.pool.ntp.org`: Kontinentspezifische NTP-Pools, die entgegen dem globalen Pool bevorzugt werden sollten, da sie geographisch nähere Server und geringere Antwortzeiten verursachen.
- `de.pool.ntp.org`, `at.pool.ntp.org`, `ch.pool.ntp.org`: Länderspezifische NTP-Pools für DACH, die noch mehr einen Antwortzeit-Vorteil als die Kontinentalpools bieten.
- `time.nist.gov`: Zeitserverpool des staatlichen USA-Metrologieinstituts NIST. Empfehlenswert nur für Verwendung innerhalb der USA.
- `ptbtime1.ptb.de`, `ptbtime2.ptb.de`, `ptbtime3.ptb.de`, `ptbtime4.ptb.de`: Zeitserver der Physikalisch-Technischen Bundesanstalt. Empfehlenswert nur für Verwendung innerhalb Deutschlands.
- Gateway: Verwendet den aktuellen Netzwerk-Gateway als NTP-Server. In vielen einfachen Heimnetzwerken fungiert der Router bzw. Gateway auch als NTP-Server (dies kann beispielsweise bei den FRITZ!Box-Routern von AVM problemlos über das Webinterface eingerichtet werden). Bei seltenem Netzwerkwechsel ist der Vorteil hiervon eine sehr geringe Antwortzeit und Netzwerklast, sowie eine einfache und flexiblere Umkonfiguration der NTP-Server, die jetzt nur noch im Router vorgenommen werden müssen.

Unabhängig von der Zeitsynchronisation kann in den Einstellungen eine Zeitzone gewählt werden. Die Zeitzonendatenbank, welche beispielsweise Informationen über Sommerzeitumstellungen enthält, wird vom Wecker nicht automatisch aktualisiert und muss durch ein Firmware-Update erneuert werden. Dank der recht seltenen Änderungen in Zeitzonenkonfigurationen ist dies nur notwendig, wenn der Wecker erkennbar von der eigentlichen Uhrzeit in einer Zeitzone abweicht.
