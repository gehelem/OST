---
title: Vorstellung
weight : 10
---

## Warum "Observatoire Sans Tête"?

Dieser Name stammt von einem Scherz mit meinem Freund Bob, bei dem es darum ging, "Headless Observatory" Wort für Wort zu übersetzen.
Dieser Begriff trifft das angestrebte Ziel gut: ein autonomer Server, der sich fernsteuern lässt.

## Was ist OST?

OST ist eine autonome Astrofotografie-Plattform. Sie läuft als Hintergrunddienst auf einer dedizierten Maschine — Raspberry Pi, Mini-PC oder virtuelle Maschine — und wird von jedem Webbrowser im lokalen Netzwerk aus bedient, ohne Bildschirm oder Tastatur an der Steuerungsmaschine.

Das Ziel ist es, eine komplette Beobachtungssitzung vom Sofa aus, aus einem anderen Gebäude oder aus der Ferne per VPN steuern zu können: Polausrichtung, Zielausrichtung, Fokussierung, Guiding, Aufnahme und Dokumentation der Nacht.

{{% notice style="warning" title="Lokales Netzwerk oder VPN — nicht für direkte Internet-Exposition" %}}
Die eingebaute Zugriffskontrolle von OST ist einfach gehalten (eine simple Berechtigungsstufe: deaktiviert, nur lesen oder voller Zugriff) und nicht dafür ausgelegt, einer feindseligen, direkt aus dem Internet erreichbaren Bereitstellung standzuhalten. Empfohlen wird der Einsatz im lokalen Netzwerk oder der Fernzugriff per VPN — nicht die direkte Exposition ins Internet ohne bewusste, informierte Sicherheitshärtung Ihrerseits. Der SSL-/Reverse-Proxy-Aufbau der öffentlichen Demo-Seite wurde geschaffen, um diese eine Demo sauber zu halten — nicht als Sicherheitsgarantie für einen echten Einsatz.
{{% /notice %}}

## Architektur

OST basiert auf einer Client-Server-Architektur:

| Komponente | Rolle |
|---|---|
| **OST-Server** | C++/Qt-Anwendung, die im Hintergrund läuft und Geräte über das INDI-Protokoll steuert |
| **Web-Oberfläche** | Angular-Client, ausgeliefert von Nginx, kommuniziert mit dem Server über WebSocket |
| **INDI-Server** | Hardware-Abstraktionsschicht — Treiber für Kameras, Montierungen, Fokussierer, Filterräder usw. |

Die Kommunikation zwischen Client und Server basiert vollständig auf JSON-Nachrichten, die über WebSocket ausgetauscht werden, was die Steuerung von OST von jedem Gerät mit einem modernen Browser aus ermöglicht.

## Module

OST ist in unabhängige Module gegliedert, jedes für eine bestimmte Funktion:

| Modul | Funktion |
|---|---|
| **Sequencer** | Automatisierte Aufnahme: Belichtungssequenzen nach Typ, Filter, Gain und Dauer |
| **Guiding** | Autoguiding über PHD2, Kontrolle von Schwankungen und Korrekturen |
| **Navigator** | Feldauflösung (Plate Solving) und automatische Zentrierung auf ein Ziel |
| **Planner** | Orchestrierung einer Objektliste: Zentrierung und Aufnahme für jedes davon |
| **Polar** | Polausrichtung nach der 3-Bilder-Methode |
| **Fokussierung** | Autofokus per V-Kurve mit HFR-Minimierung |
| **Inspector** | Analyse der optischen Qualität: HFR-Karte, Aberrationskarte, Eckenmosaik |
| **IndiPanel** | Rohes Kontrollfeld für alle verbundenen INDI-Geräte |
| **Allsky** | Weitwinkelkamera: Keogramm, Stack, Timelapse und Wetterüberlagerung |

## Inspirationen

In seiner Form lässt sich OST von zahlreichen anderen Open-Source-Projekten inspirieren:
- https://github.com/GuLinux/AstroPhoto-Plus
- https://github.com/pludov/mobindi
- https://www.indigo-astronomy.org/for-users.html

Natürlich kommt die Inspiration auch von kommerziellen Lösungen:
- ASIAIR
- Stellarmate

## Verwendete Technologien

| Bereich | Technologie |
|---|---|
| Server | C++17, Qt6, libnova, cfitsio, libindi, StellarSolver |
| Web-Oberfläche | Angular, TypeScript |
| Webserver | Nginx |
| Protokoll | INDI, WebSocket, JSON |
| Video | ffmpeg (Allsky-Timelapse) |
| Packaging | Ubuntu-PPA (Launchpad) |
