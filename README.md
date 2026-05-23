[![CMake](https://github.com/gehelem/OST/actions/workflows/cmake.yml/badge.svg)](https://github.com/gehelem/OST/actions/workflows/cmake.yml)
[![GitLab build](https://gitlab.ostserver.fr/gilles/OST/badges/main/pipeline.svg?key_text=build&key_width=40)](https://gitlab.ostserver.fr/gilles/OST/-/pipelines)
[![GitLab test](https://gitlab.ostserver.fr/gilles/OST/badges/main/pipeline.svg?key_text=test&key_width=35)](https://gitlab.ostserver.fr/gilles/OST/-/pipelines)

See documentation site :

https://documentation.ostserver.fr/

# OST - Observatoire Sans Tête

> /!\ Work in progress — functional but evolving fast.

"Observatoire Sans Tête" is a joke with my friend Bob, a word-for-word French translation of "Headless Observatory".
The goal: a server you control remotely from any browser, with no screen or keyboard attached to the machine running it.

OST is inspired by great open-source projects like [AstroPhoto Plus](https://astrophotoplus.gulinux.net/) and [MobIndi](https://github.com/pludov/mobindi), and by commercial solutions like ASIAIR and Stellarmate.
The main difference: it's written in C++/Qt, talks to hardware through INDI, and exposes everything over WebSocket + JSON to any browser-based frontend.

## What it does

OST runs as a background service and orchestrates a full astrophotography session:

| Module | Function |
|---|---|
| **Focus** | Autofocus via V-curve and HFR minimisation (SCXML state machine) |
| **Guider** | Autoguiding via PHD2, drift monitoring and correction |
| **Sequencer** | Automated acquisition sequences (frame type, filter, gain, exposure) |
| **Navigator** | Plate solving (StellarSolver) and automatic field centering |
| **Planner** | Orchestrates Navigator + Sequencer for a list of objects |
| **Polar** | Polar alignment via the 3-image method |
| **Inspector** | Optical quality analysis: HFR map, aberration map, corner mosaic |
| **Allsky** | All-sky camera: keogram, max stacking, timelapse, weather overlay |
| **IndiPanel** | Raw control panel for all connected INDI devices |

Modules communicate with each other (e.g. Sequencer triggers autofocus, suspends guiding during focus).

## Architecture

- **Backend**: C++17 / Qt, communicates with hardware via INDI protocol
- **Frontend**: Angular web app, served by Nginx, talks to the backend over WebSocket
- **Protocol**: JSON messages over WebSocket
- **Packaging**: Ubuntu PPA (Launchpad) for 22.04 and 24.04

## Installing from PPA

```shell
sudo add-apt-repository ppa:gehelem/ostserver-daily
sudo apt update
sudo apt install ostserver
```

## Building from source

```shell
sudo add-apt-repository ppa:mutlaqja/ppa
sudo apt update
sudo apt install build-essential cmake git \
    libstellarsolver-dev libeigen3-dev libcfitsio-dev \
    zlib1g-dev libindi-dev libnova-dev libgsl-dev libraw-dev \
    wcslib-dev libqt5websockets5-dev libqt5svg5-dev \
    libqt5scxml5-dev qttools5-dev-tools qtdeclarative5-dev \
    libsecret-1-dev libavahi-client-dev libavahi-common-dev \
    nginx indi-bin

mkdir build && cd build
cmake ..
make -j$(nproc)
```

If running headless (no display), add this to `.bashrc`:
```shell
export QT_QPA_PLATFORM=offscreen
```

## Running

```shell
ostserver --webroot=/path/to/media --libpath=/path/to/modules \
          --lng=fr --loglevel=2 --grant=0
```

Then open `http://<your-machine-ip>` in any browser.

## Platforms

Tested on Raspberry Pi 4, small NUCs, and VMs (VirtualBox/VMware) running Ubuntu 22.04 and 24.04.
Any Linux supporting Qt5 + INDI should work.
