---
title: Den ostserver-Dienst verwalten
weight: 30
description: "ostserver starten, stoppen und die Startoptionen des Dienstes anpassen"
---

`ostserver` läuft im Hintergrund als **systemd-Dienst**. Systemd ist der Dienstmanager von Linux: Es startet `ostserver` automatisch beim Hochfahren der Maschine, startet ihn bei einem Absturz neu und erlaubt die Steuerung über einfache Befehle.

---

## Starten, stoppen, neu starten

```bash
# Dienst starten
sudo systemctl start ostserver

# Dienst stoppen
sudo systemctl stop ostserver

# Dienst neu starten (nützlich nach Änderung der Optionen)
sudo systemctl restart ostserver

# Status des Dienstes anzeigen
sudo systemctl status ostserver
```

---

## Automatischen Start aktivieren / deaktivieren

Standardmäßig ist der Dienst **aktiviert**: Er startet automatisch bei jedem Hochfahren der Maschine.

```bash
# Automatischen Start deaktivieren
sudo systemctl disable ostserver

# Automatischen Start wieder aktivieren
sudo systemctl enable ostserver
```

---

## Logs einsehen

```bash
# Die letzten Logs anzeigen
sudo journalctl -u ostserver

# Logs in Echtzeit verfolgen
sudo journalctl -u ostserver -f

# Auf die letzten 100 Zeilen begrenzen
sudo journalctl -u ostserver -n 100
```

---

## Startoptionen anpassen

Bei der Installation wird eine Konfigurationsdatei im Home-Verzeichnis des Benutzers erstellt:

```
~/.ost/ostserver.parms
```

Diese Datei enthält **ein Argument pro Zeile**. Zeilen, die mit `#` beginnen, sind Kommentare und werden ignoriert. Leerzeichen in Werten werden nativ unterstützt — Anführungszeichen sind nicht nötig.

Zum Bearbeiten:

```bash
nano ~/.ost/ostserver.parms
```

Beispielinhalt:

```
# Web server media folder (must be writable)
--webroot=/var/lib/osterix/media

# Start embedded INDI server (Y/N)
--indiserver=Y

# Log level: 0=debug, 1=info, 2=warning, 3=error, 4=critical
--loglevel=1

# Security level: 0=disabled, 1=read only, 2=user grants
--grant=0

# Frontend banner text (spaces are allowed, no quoting needed)
--banner=Mein Observatorium ohne Kopf
```

Nach jeder Änderung den Dienst neu starten, damit sie wirksam wird:

```bash
sudo systemctl restart ostserver
```

{{% notice style="info" title="Datei bleibt bei Updates erhalten" %}}
Die Datei `ostserver.parms` wird bei einem Paket-Update **niemals überschrieben**. Ihre Anpassungen bleiben erhalten.
{{% /notice %}}

---

## Referenz der verfügbaren Optionen

| Option | Standard | Beschreibung |
|---|---|---|
| `--webroot=<Pfad>` | `/var/lib/osterix/media` | Von nginx ausgeliefertes Medienverzeichnis |
| `--indiserver=Y\|N` | `N` | Eingebetteten INDI-Server starten |
| `--loglevel=<0-4>` | `1` | Log-Verbosität: 0=debug, 1=info, 2=warning, 3=error, 4=critical |
| `--logfile=<Pfad>` | *(zeitgestempelt in `~/.ost/`)* | Pfad der Log-Datei |
| `--grant=<0\|1\|2>` | — | Sicherheit: 0=deaktiviert, 1=nur Lesen, 2=Benutzer-Grants |
| `--lng=<fr\|en\|...>` | `fr` | Sprache der Oberfläche (fr, en, de, it, es) |
| `--banner=<Text>` | `Observatoire Sans tête` | Im Frontend angezeigtes Banner |
| `--configuration=<Name>` | `default` | Beim Start zu ladende Konfiguration |
| `--libpath=<Pfad>` | *(Systempfad)* | Verzeichnis der `.so`-Module |
| `--dbpath=<Pfad>` | *(Arbeitsverzeichnis)* | Speicherort der Datenbank |
| `--systemwatchinterval=<s>` | `10` | Intervall der Systemüberwachung in Sekunden (0 zum Deaktivieren) |
| `--minfree=<0-99>` | `10` | Minimal freier Speicherplatz in % vor Überspringen des Bild-Schreibens |
| `--ssl=Y\|N` | `N` | SSL für das WebSocket aktivieren |
| `--sslcert=<Pfad>` | `/etc/ost/server.crt` | SSL-Zertifikat |
| `--sslkey=<Pfad>` | `/etc/ost/server.key` | SSL-Privatschlüssel |
| `--setadminpassword=<Passwort>` | — | Administrator-Passwort festlegen (nach Gebrauch entfernen) |
