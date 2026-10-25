---
title: Installation über das Ubuntu-PPA (Launchpad)
weight: 20
description: "Vollständiges Vorgehen unter Ubuntu 26.04 LTS über das offizielle PPA-Repository"
---

Voraussetzungen: frisches Ubuntu Server 26.04 (Resolute), sudo-Zugriff, Internetverbindung.

Diese Methode nutzt das offizielle Launchpad-PPA — `ostserver-complete` wird über `apt` installiert und aktualisiert sich automatisch mit `apt upgrade`.

{{% notice style="note" title="22.04 / 24.04?" %}}
Das PPA veröffentlicht derzeit nur für Ubuntu 26.04 — unter 22.04 (Jammy) oder 24.04 (Noble) stattdessen das [manuelle .deb-Verfahren]({{% relref "fresh.de.md" %}}) verwenden.
{{% /notice %}}

---

## 1. System aktualisieren

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. INDI installieren

INDI ist die Hardware-Abstraktionsschicht, die Kameras, Montierungen, Fokussierer und Filterräder steuert. `ostserver` benötigt `libindi1`, und die meisten Kameras (ZWO, QHY, PlayerOne und andere) brauchen ihren Treiber aus `indi-3rdparty-drivers`.

```bash
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo add-apt-repository -y universe
sudo apt update
sudo apt install -y libindi1 indi-bin gsc indi-3rdparty-libs indi-3rdparty-drivers
```

---

## 3. Astrometry.net-Indizes installieren

Erforderlich für Plate Solving (Module Navigator und Polar).

```bash
sudo apt install -y astrometry.net
```

Die Indexdateien sind nicht über apt verfügbar — manuell von `http://data.astrometry.net/` herunterladen.

{{% notice style="info" title="Die richtigen Indizes wählen" %}}
Laden Sie nur die zu Ihrem Setup passenden Indizes herunter. Als Faustregel: die Indizes 4208–4210 decken weite Felder ab. Für längere Brennweiten 4206–4207 ergänzen. Jeder Index ist mehrere Hundert MB groß.
{{% /notice %}}

Die zu Ihrem Setup passenden Zeilen auskommentieren:

```bash
# --- Wide field > 5° (wide-angle, short focal length) ---
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4214.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4213.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4212.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4211.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4210.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4209.fits
sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4208.fits

# --- Medium field 1°–5° (long focal length) — 12 files per index ---
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-00.fits
#...
#sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-11.fits
```

Um alle Indizes auf einmal herunterzuladen (mehrere GB):

```bash
for idx in 4208 4209 4210 4211 4212 4213 4214; do
    sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}.fits
done
for idx in 4206 4207; do
    for i in $(seq -w 0 11); do
        sudo wget -nc -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}-${i}.fits
    done
done
```

`-nc` (no-clobber) überspringt eine bereits heruntergeladene Datei — dieser Abschnitt kann so bedenkenlos erneut ausgeführt werden, um einen unvollständigen Satz zu ergänzen.

---

## 4. Systemabhängigkeiten

Das Paket `ostserver-complete` benötigt mehrere Qt6-, GSL-, cfitsio-, libnova- und StellarSolver-Bibliotheken sowie nginx.
`apt` löst diese automatisch, stellen Sie aber sicher, dass das `universe`-Repository aktiviert ist:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` ist direkt in das Paket eingebettet — es gibt noch keine eigenständig veröffentlichte Qt6-Version.

---

## 5. ostserver-complete installieren

Das offizielle OST-PPA hinzufügen und `ostserver-complete` installieren — Backend, alle Module, die Web-UI und die nginx-Verknüpfung dazwischen, alles in einem Paket:

```bash
sudo add-apt-repository ppa:gehelem/ostserver -y
sudo apt update
sudo apt install -y ostserver-complete
```

`apt` löst automatisch alle Abhängigkeiten, nginx eingeschlossen.

Das Paket installiert:
- `/usr/bin/ostserver` — die Hauptbinärdatei
- `/usr/lib/libost*.so` — die Module (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)
- die statischen Dateien der Web-UI, mit einer bereits eingerichteten nginx-Site zu deren Auslieferung

{{% notice style="info" title="Systemd-Dienst inklusive" %}}
Das Paket installiert und aktiviert automatisch den Dienst `ostserver`. Der Dienst läuft unter dem Benutzer, der `sudo apt install` ausgeführt hat, und das Verzeichnis `/var/lib/osterix/media` wird initialisiert.
{{% /notice %}}

{{% notice style="info" title="WebSocket-Port" %}}
`ostserver` lauscht per WebSocket auf Port **9624**.
{{% /notice %}}

---

## 6. Installation überprüfen

```bash
# Service status
sudo systemctl status ostserver
sudo systemctl status nginx

# Listening ports
ss -tlnp | grep -E '80|9624|7624'

# Live ostserver logs
sudo journalctl -u ostserver -f

# Test Nginx config
sudo nginx -t
```

---

## 7. Erster Start

Über einen Browser im lokalen Netzwerk: `http://<machine-ip>/`

1. Den INDI-Server mit Ihren Treibern starten
2. Die OST-Oberfläche öffnen → `http://<ip>/`

Die Module sind über das Hauptmenü erreichbar.

---

## Übersicht der Dienste

| Service | Rolle | Port |
|---|---|---|
| `ostserver` | Backend C++/Qt, Astrofoto-Orchestrierung | 9624 (WebSocket) |
| `nginx` | Liefert die Web-UI aus | 80 |
| `indiserver` | INDI-Hardware-Treiberschicht | 7624 |

---

## Aktualisierung

```bash
sudo apt update && sudo apt upgrade -y
```

`ostserver-complete` aktualisiert sich über `apt upgrade` wie jedes andere Systempaket — nginx und die statischen Dateien der Web-UI werden dabei mit aktualisiert.

---

## Deinstallation

```bash
sudo apt remove ostserver-complete
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
