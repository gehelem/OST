---
title: Presentation
weight : 10
---

## Why "Observatoire Sans Tête"?

This name comes from a joke with my friend Bob, which consists of translating word for word "Headless Observatory".
This term captures the intended goal: an autonomous server that can be controlled remotely.

## What is OST?

OST is an autonomous astrophotography platform. It runs as a background service on a dedicated machine — Raspberry Pi, mini-PC, or virtual machine — and is operated from any web browser on the local network, with no screen or keyboard attached to the control machine.

The goal is to run a complete observation session from a couch, another room, or remotely over VPN: polar alignment, pointing, focusing, guiding, acquisition, and night documentation.

## Architecture

OST is built on a client-server architecture:

| Component | Role |
|---|---|
| **OST server** | C++/Qt application running in the background, drives devices via the INDI protocol |
| **Web interface** | Angular client served by Nginx, communicates with the server over WebSocket |
| **INDI server** | Hardware abstraction layer — drivers for cameras, mounts, focusers, filter wheels, etc. |

Communication between client and server is entirely based on JSON messages exchanged over WebSocket, allowing OST to be controlled from any device with a modern browser.

## Modules

OST is organised into independent modules, each dedicated to a specific function:

| Module | Function |
|---|---|
| **Sequencer** | Automated acquisition: sequences of frames by type, filter, gain and duration |
| **Guider** | Autoguiding via PHD2, drift monitoring and correction control |
| **Navigator** | Plate solving and automatic centering on a target |
| **Planner** | Orchestration of an object list: centering then acquisition for each entry |
| **Polar** | Polar alignment using the 3-image method |
| **Focus** | Autofocus via V-curve with HFR minimisation |
| **Inspector** | Optical quality analysis: HFR map, aberration map, corner mosaic |
| **IndiPanel** | Raw control panel for all connected INDI devices |
| **Allsky** | Wide-angle camera: keogram, stacking, timelapse and weather overlay |

## Inspirations

OST draws on many other open-source projects:
- https://github.com/GuLinux/AstroPhoto-Plus
- https://github.com/pludov/mobindi
- https://www.indigo-astronomy.org/for-users.html

Of course, inspiration also comes from commercial solutions:
- ASIAIR
- Stellarmate

## Technologies

| Area | Technology |
|---|---|
| Server | C++17, Qt6, libnova, cfitsio, libindi, StellarSolver |
| Web interface | Angular, TypeScript |
| Web server | Nginx |
| Protocol | INDI, WebSocket, JSON |
| Video | ffmpeg (allsky timelapse) |
| Packaging | Ubuntu PPA (Launchpad) |
