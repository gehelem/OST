---
title: Installing from the Ubuntu PPA (Launchpad)
weight: 20
description: "Full procedure on Ubuntu 26.04 LTS using the official PPA repository"
---

Prerequisites: fresh Ubuntu Server 26.04 (Resolute), sudo access, Internet connection.

This method uses the official Launchpad PPA — `ostserver-complete` is installed via `apt` and updates automatically with `apt upgrade`.

{{% notice style="note" title="22.04 / 24.04?" %}}
The PPA currently only publishes for Ubuntu 26.04 — on 22.04 (Jammy) or 24.04 (Noble), use the [manual .deb procedure]({{% relref "fresh.en.md" %}}) instead.
{{% /notice %}}

---

## 1. System update

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Install INDI

INDI is the hardware abstraction layer driving cameras, mounts, focusers and filter wheels. `ostserver` depends on `libindi1`, and most cameras (ZWO, QHY, PlayerOne and others) need their driver from `indi-3rdparty-drivers`.

```bash
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo add-apt-repository -y universe
sudo apt update
sudo apt install -y libindi1 indi-bin gsc indi-3rdparty-libs indi-3rdparty-drivers
```

---

## 3. Install Astrometry.net indexes

Required for plate solving (Navigator and Polar modules).

```bash
sudo apt install -y astrometry.net
```

Index files are not available via apt — download them manually from `http://data.astrometry.net/`.

{{% notice style="info" title="Choosing the right indexes" %}}
Only download indexes suited to your setup. As a starting point: indexes 4208–4210 cover wide fields. Add 4206–4207 for longer focal lengths. Each index is several hundred MB.
{{% /notice %}}

Uncomment the lines matching your setup:

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

To download all indexes at once (several GB):

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

`-nc` (no-clobber) skips a file already downloaded — re-run this section safely to top up a partial set.

---

## 4. System dependencies

The `ostserver-complete` package requires several Qt6, GSL, cfitsio, libnova and StellarSolver libraries, plus nginx.
`apt` will resolve them automatically, but make sure the `universe` repository is enabled:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` is bundled directly in the package — no Qt6 version has been published independently yet.

---

## 5. Install ostserver-complete

Add the official OST PPA and install `ostserver-complete` — backend, every module, the web UI and the nginx wiring between them, all in one package:

```bash
sudo add-apt-repository ppa:gehelem/ostserver -y
sudo apt update
sudo apt install -y ostserver-complete
```

`apt` resolves all dependencies automatically, nginx included.

The package installs:
- `/usr/bin/ostserver` — the main binary
- `/usr/lib/libost*.so` — the modules (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)
- the web UI's static files, with an nginx site already wired to serve them

{{% notice style="info" title="systemd service included" %}}
The package automatically installs and enables the `ostserver` service. The service runs as the user who invoked `sudo apt install`, and the `/var/lib/osterix/media` directory is initialized.
{{% /notice %}}

{{% notice style="info" title="WebSocket port" %}}
`ostserver` listens for WebSocket connections on port **9624**.
{{% /notice %}}

---

## 6. Verify the installation

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

## 7. First start

From a browser on the local network: `http://<machine-ip>/`

1. Start the INDI server with your drivers
2. Open the OST interface → `http://<ip>/`

Modules are accessible from the main menu.

---

## Services summary

| Service | Role | Port |
|---|---|---|
| `ostserver` | C++/Qt back-end, astrophoto orchestration | 9624 (WebSocket) |
| `nginx` | Serves the web UI | 80 |
| `indiserver` | INDI hardware driver layer | 7624 |

---

## Update

```bash
sudo apt update && sudo apt upgrade -y
```

`ostserver-complete` updates via `apt upgrade` like any system package — nginx and the web UI's static files come along with it.

---

## Uninstall

```bash
sudo apt remove ostserver-complete
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
