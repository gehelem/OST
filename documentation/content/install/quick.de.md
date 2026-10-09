---
title: Schnellinstallation (Ubuntu Resolute)
weight: 5
description: "Der schnellste Weg, OST zum Laufen zu bringen: eine frische Ubuntu-26.04-Maschine, ein Skript"
---

{{% notice style="tip" title="Am besten für Einsteiger" %}}
Noch nie eine Linux-Kommandozeile benutzt? Diese Seite ist genau richtig
für Sie. Folgen Sie einfach den drei Schritten unten, in der angegebenen
Reihenfolge.
{{% /notice %}}

Dieses einzige Skript installiert alles Nötige auf einer Maschine: das
OST-Backend, die Weboberfläche, nginx, INDI, `astrometry.net` und die
Dokumentation, die Sie gerade lesen.

{{% notice style="note" title="Verwenden Sie stattdessen Arch Linux oder Fedora?" %}}
Diese Seite ist speziell für Ubuntu. Sehen Sie stattdessen die
[Schnellinstallation (Arch Linux)](../quick-arch/) oder die
[Schnellinstallation (Fedora)](../quick-fedora/).
{{% /notice %}}

---

## 1. Frisches Ubuntu 26.04 installieren

Installieren Sie **Ubuntu Server 26.04 LTS („Resolute")** auf der
Maschine, die OST ausführen soll — ein Raspberry Pi, ein Mini-PC oder
eine virtuelle Maschine. Verwenden Sie den Standard-Ubuntu-Installer;
keine besondere Option ist nötig.

{{% notice style="note" title="Warum genau 26.04?" %}}
Dieses Skript lädt das für Ubuntu 26.04 gebaute Paket herunter. Die
anderen unterstützten Versionen (22.04, 24.04) benötigen ein anderes,
manuelleres Vorgehen — siehe stattdessen die
[vollständige Installationsanleitung](../fresh/).
{{% /notice %}}

Sobald Ubuntu installiert ist, öffnen Sie ein Terminal (direkt auf der
Maschine oder per SSH von einem anderen Computer aus).

---

## 2. Installationsskript herunterladen

Kopieren Sie den folgenden Befehl, fügen Sie ihn ins Terminal ein und
drücken Sie Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete.sh
```

Wenn alles funktioniert, passiert auf dem Bildschirm nichts — das ist
normal. Eine neue Datei, `install-latest-complete.sh`, befindet sich
jetzt in Ihrem aktuellen Ordner.

---

## 3. Skript ausführen

```bash
chmod +x install-latest-complete.sh
./install-latest-complete.sh
```

Das Skript erledigt dann alles von selbst. Ein paar Dinge, mit denen Sie
rechnen sollten:

- Es fragt mehrmals nach **Ihrem Passwort** (`sudo`) — das ist normal,
  es braucht die Berechtigung, um Systempakete zu installieren.
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
Standard-Ordnerstruktur, nginx als Webserver, nur Ubuntu 26.04). Wenn
Sie etwas anderes benötigen, folgen Sie stattdessen der
[vollständigen Installationsanleitung](../fresh/) — gleiches Ergebnis,
aber jeder Schritt erklärt und anpassbar.
{{% /notice %}}
