---
title: Schnellinstallation (Fedora)
weight: 7
description: "Der schnellste Weg, OST auf Fedora zum Laufen zu bringen: ein Skript"
---

{{% notice style="tip" title="Am besten für Einsteiger" %}}
Noch nie eine Linux-Kommandozeile benutzt? Diese Seite ist genau richtig
für Sie. Folgen Sie einfach den drei Schritten unten, in der angegebenen
Reihenfolge.
{{% /notice %}}

Dieses einzige Skript installiert alles Nötige auf einer Maschine: das
OST-Backend, die Weboberfläche, nginx, INDI, `astrometry.net` und die
Dokumentation, die Sie gerade lesen.

{{% notice style="note" title="Verwenden Sie stattdessen Ubuntu oder Arch?" %}}
Diese Seite ist speziell für Fedora. Sehen Sie stattdessen die
[Schnellinstallation (Ubuntu Resolute)](../quick/) oder die
[Schnellinstallation (Arch Linux)](../quick-arch/).
{{% /notice %}}

---

## 1. Frisches Fedora installieren

Installieren Sie **Fedora Server** (oder Workstation) auf der
Maschine, die OST ausführen soll — ein Raspberry Pi, ein Mini-PC oder
eine virtuelle Maschine. Verwenden Sie den Standard-Fedora-Installer;
keine besondere Option ist nötig.

Sobald Fedora installiert ist, öffnen Sie ein Terminal (direkt auf der
Maschine oder per SSH von einem anderen Computer aus).

---

## 2. Installationsskript herunterladen

Kopieren Sie den folgenden Befehl, fügen Sie ihn ins Terminal ein und
drücken Sie Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-fedora.sh
```

Wenn alles funktioniert, passiert auf dem Bildschirm nichts — das ist
normal. Eine neue Datei, `install-latest-complete-fedora.sh`, befindet
sich jetzt in Ihrem aktuellen Ordner.

---

## 3. Skript ausführen

```bash
chmod +x install-latest-complete-fedora.sh
./install-latest-complete-fedora.sh
```

Das Skript erledigt dann alles von selbst. Ein paar Dinge, mit denen Sie
rechnen sollten:

- Es fragt mehrmals nach **Ihrem Passwort** (`sudo`) — das ist normal,
  es braucht die Berechtigung, um Systempakete zu installieren.
- Es öffnet die Ports für die Web-Oberfläche (80) und WebSocket (9624)
  in `firewalld`, das auf Fedora standardmäßig aktiv ist (anders als
  bei Ubuntu/Arch) und sonst beide für jede andere Maschine im
  Netzwerk blockieren würde.
- Es entfernt die von nginx auf Fedora mitgelieferte Standard-Seite
  (fest eingebaut in `/etc/nginx/nginx.conf` selbst) - sonst würde sie
  stillschweigend über OSTs eigene Seite auf Port 80 gewinnen.
- Es installiert `gsc` (realistische Sternfelddaten nur für den
  CCD-*Simulator* von INDI, für echte Hardware nie benötigt) aus einem
  eigenen vorgebauten Paket - für Fedora gibt es dafür kein offizielles
  Paket.
- Möglicherweise erscheint ein blauer Bildschirm, der fragt, welche
  zusätzlichen `astrometry.net`-Indexdateien Sie zu den
  Standard-Dateien herunterladen möchten. Verwenden Sie die
  **Pfeiltasten** zum Bewegen, die **Leertaste** zum An-/Abwählen und
  **Tab**, dann **Enter** zum Bestätigen. Wenn Sie nicht sicher sind,
  lassen Sie alles abgewählt und drücken Sie Enter — Sie können die
  Indizes jederzeit später hinzufügen.
- Es gibt Fortschrittsmeldungen aus, während es herunterlädt und
  installiert. Das kann je nach Internetverbindung einige Minuten
  dauern.

Ganz am Ende gibt es zwei Adressen aus, zum Beispiel:

```
Web UI: http://192.168.1.42/
Documentation: http://192.168.1.42/documentation/
```

---

## Fertig

Öffnen Sie die erste Adresse (`Web UI: ...`) in einem Webbrowser, von
einem beliebigen Gerät im selben lokalen Netzwerk wie die Maschine, auf
der Sie OST gerade installiert haben. Sie sollten die OST-Oberfläche
sehen.

{{% notice style="info" title="Mehr Kontrolle gewünscht?" %}}
Dieses Skript trifft Entscheidungen für Sie (SSL deaktiviert,
Standard-Ordnerstruktur, nginx als Webserver). Wenn Sie etwas anderes
benötigen, installieren Sie das Paket `ostserver`/`ostserver-complete`
stattdessen von Hand (das vorgefertigte `.rpm` auf der Releases-Seite)
und richten Sie den Rest selbst ein — die eigenen Schritte des Skripts
dienen dabei als Anleitung für das manuelle Vorgehen.
{{% /notice %}}
