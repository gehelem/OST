---
title: Installation aus den .deb-Paketen
weight: 10
description: "Vollständiges Vorgehen unter Ubuntu 22.04 / 24.04 / 26.04 LTS mit den .deb-Paketen von GitLab"
---

Voraussetzungen: frisches Ubuntu Server 22.04 (Jammy), 24.04 (Noble) oder 26.04 (Resolute), sudo-Zugriff, Internetverbindung.

---

## 1. System aktualisieren

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. INDI installieren

INDI ist die Hardware-Abstraktionsschicht, die Kameras, Montierungen, Fokussierer und Filterräder steuert. `ostserver` benötigt `libindi1`.

{{% notice style="note" title="Warum nicht das mutlaqja-PPA?" %}}
Seit August 2026 veröffentlicht das PPA `ppa:mutlaqja/ppa` nur noch für Ubuntu 26.04. OST stellt daher einen festen Satz INDI-2.2.4-Pakete — zusammen mit `libxisf` und `gsc` — in seinem eigenen Registry für alle drei LTS-Versionen bereit.
{{% /notice %}}

Laden Sie das zu Ihrer Ubuntu-Version passende Archiv herunter und installieren Sie dessen Inhalt:

```bash
# Je nach Version auswählen:
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)
# INDI_TAR=indi-deps-resolute.tar # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt install -y ./indi-deps/*.deb
```

`apt` löst die restlichen Abhängigkeiten (`libgsl`, `liberfa`, `libpugixml` …) automatisch über das `universe`-Repository — stellen Sie sicher, dass es aktiviert ist (siehe §4).

Das Paket `gsc` (Global Star Catalog, vom INDI-CCD-Simulator zur Erzeugung eines realistischen Sternfelds verwendet) ist im Archiv enthalten — kein manueller Schritt nötig.


---

## 3. Astrometry.net-Indizes installieren

Erforderlich für Plate Solving (Module Navigator und Polar).

```bash
sudo apt install -y astrometry.net
```

Die Indexdateien sind nicht über apt verfügbar — manuell von `http://data.astrometry.net/` herunterladen.

{{% notice style="info" title="Die richtigen Indizes wählen" %}}
Laden Sie nur die zu Ihrem Setup passenden Indizes herunter. Als Faustregel: mit den Indizes 4208–4210 (weite Felder) beginnen, dann 4206–4207 ergänzen, falls Sie eine lange Brennweite haben. Die Indizes sind groß — jeweils mehrere Hundert MB.
{{% /notice %}}

Die zu Ihrem Setup passenden Zeilen auskommentieren:

```bash
# --- Weite Felder > 5° (Weitwinkel, kurze Brennweite) ---
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4214.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4213.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4212.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4211.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4210.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4209.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4208.fits

# --- Mittlere Felder 1°–5° (lange Brennweite) — 12 Dateien pro Index ---
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-00.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-01.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-02.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-03.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-04.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-05.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-06.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-07.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-08.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-09.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-10.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-11.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-00.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-01.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-02.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-03.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-04.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-05.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-06.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-07.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-08.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-09.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-10.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-11.fits
```

Um alle Indizes auf einmal herunterzuladen (mehrere GB):

```bash
for idx in 4208 4209 4210 4211 4212 4213 4214; do
    sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}.fits
done
for idx in 4206 4207; do
    for i in $(seq -w 0 11); do
        sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}-${i}.fits
    done
done
```

---

## 4. Systemabhängigkeiten

Das Paket `ostserver` benötigt mehrere Qt6-Bibliotheken, GSL, cfitsio, libnova und StellarSolver.
`apt` löst diese automatisch, stellen Sie aber sicher, dass das `universe`-Repository aktiviert ist:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` ist direkt in das Paket `ostserver` eingebettet — es gibt noch keine eigenständig veröffentlichte Qt6-Version.

---

## 5. Backend installieren: ostserver

Die neueste Release vom GitLab OST herunterladen, dabei das zur Ubuntu-Version passende Paket wählen:

```bash
DEB=ostserver_latest_2404_amd64.deb      # Ubuntu 24.04 (Noble)
# DEB=ostserver_latest_2204_amd64.deb    # Ubuntu 22.04 (Jammy)
# DEB=ostserver_latest_2604_amd64.deb    # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
sudo apt install -y "./${DEB}"
```

`apt` löst automatisch alle Abhängigkeiten.

Das Paket installiert:
- `/usr/bin/ostserver` — die Hauptbinärdatei
- `/usr/lib/libost*.so` — die Module (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)

{{% notice style="info" title="Systemd-Dienst inklusive" %}}
Das Paket installiert und aktiviert automatisch den Dienst `ostserver`. Der Dienst läuft unter dem Benutzer, der `sudo apt install` ausgeführt hat, und das Verzeichnis `/var/lib/osterix/media` wird initialisiert.
{{% /notice %}}

{{% notice style="info" title="WebSocket-Port" %}}
`ostserver` lauscht per WebSocket auf Port **9624**.
{{% /notice %}}

---

## 6. Frontend installieren: osterix

Die neueste Release vom GitLab OST herunterladen:

```bash
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
```

Das Paket installieren (nginx wird automatisch mitinstalliert):

```bash
sudo apt install -y ./osterix-front_latest.deb
```

Das Paket installiert automatisch:
- Die statischen Angular-Dateien in `/var/www/osterix/`
- Die Nginx-Konfiguration in `/etc/nginx/sites-available/osterix`
- Den symbolischen Link `/etc/nginx/sites-enabled/osterix`
- Lädt Nginx neu

Vom Paket abgelegte Nginx-Konfiguration:

```nginx
server {
    listen 80;
    root /var/www/osterix;
    index index.html;
    server_name _;

    location / {
        try_files $uri $uri/ /index.html;
    }

    location /ostmedia/ {
        alias /var/lib/osterix/media/;
        autoindex on;
    }
}
```

---

## 7. Installation überprüfen

```bash
# Status der Dienste
sudo systemctl status ostserver
sudo systemctl status nginx

# Lauschende Ports
ss -tlnp | grep -E '80|9624|7624'

# ostserver-Logs in Echtzeit
sudo journalctl -u ostserver -f

# Nginx-Konfiguration testen
sudo nginx -t
```

---

## 8. Erster Start

Über einen Browser im lokalen Netzwerk: `http://<ip-der-maschine>/`

1. Den INDI-Server mit Ihren Treibern starten
2. Die OST-Oberfläche öffnen → `http://<ip>/`

Die Module sind über das Hauptmenü erreichbar.

---

## Übersicht der Dienste

| Dienst | Rolle | Port |
|---|---|---|
| `ostserver` | Backend C++/Qt, Astrofoto-Orchestrierung | 9624 (WebSocket) |
| `nginx` | Liefert das Angular-Frontend aus | 80 |
| `indiserver` | INDI-Hardware-Treiberschicht | 7624 |

---

## Aktualisierung

```bash
DEB=ostserver_latest_2404_amd64.deb      # Ubuntu 24.04 (Noble)
# DEB=ostserver_latest_2204_amd64.deb    # Ubuntu 22.04 (Jammy)
# DEB=ostserver_latest_2604_amd64.deb    # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
sudo apt install -y "./${DEB}" ./osterix-front_latest.deb
sudo systemctl restart ostserver
```

Nginx lädt sich über das postinst-Skript des osterix-Pakets automatisch neu.

---

## Deinstallation

```bash
sudo apt remove ostserver osterix-front
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
