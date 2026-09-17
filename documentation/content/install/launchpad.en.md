---
title: Installing from the Ubuntu PPA (Launchpad)
weight: 20
description: "Full procedure on Ubuntu 22.04 / 24.04 / 26.04 LTS using the official PPA repository"
---

Prerequisites: fresh Ubuntu Server 22.04 (Jammy), 24.04 (Noble) or 26.04 (Resolute), sudo access, Internet connection.

This method uses the official Launchpad PPA — `ostserver` is installed via `apt` and updates automatically with `apt upgrade`.

---

## 1. System update

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Install INDI

INDI is the hardware abstraction layer driving cameras, mounts, focusers and filter wheels. `ostserver` depends on `libindi1`.

{{% notice style="note" title="Why not the mutlaqja PPA?" %}}
Since August 2026 the `ppa:mutlaqja/ppa` PPA only publishes for Ubuntu 26.04. OST therefore ships a frozen set of INDI 2.2.4 packages — together with `libxisf` and `gsc` — in its own registry, for all three LTS releases.
{{% /notice %}}

Download the archive matching your Ubuntu release and install its contents:

```bash
# Pick the one for your release:
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)
# INDI_TAR=indi-deps-resolute.tar # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt install -y ./indi-deps/*.deb
```

`apt` pulls the remaining runtime dependencies (`libgsl`, `liberfa`, `libpugixml`…) from the `universe` pocket — make sure it is enabled (see §4).

The `gsc` package (Global Star Catalog, used by INDI's CCD simulator to render a realistic star field) is included in the archive — no manual step.

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
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4214.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4213.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4212.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4211.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4210.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4209.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4208.fits

# --- Medium field 1°–5° (long focal length) — 12 files per index ---
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-00.fits
#...
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-11.fits
```

---

## 4. System dependencies

The `ostserver` package requires several Qt6, GSL, cfitsio, libnova and StellarSolver libraries.
`apt` will resolve them automatically, but make sure the `universe` repository is enabled:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` is bundled directly in the `ostserver` package — no Qt6 version has been published independently yet.

---

## 5. Install the back-end: ostserver

Add the official OST PPA and install `ostserver`:

```bash
sudo add-apt-repository ppa:gehelem/ostserver -y
sudo apt update
sudo apt install -y ostserver
```

`apt` resolves all dependencies automatically. The package is available for Ubuntu 22.04, 24.04 and 26.04 — `apt` automatically installs the correct version for your system.

The package installs:
- `/usr/bin/ostserver` — the main binary
- `/usr/lib/libost*.so` — the modules (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)

{{% notice style="info" title="systemd service included" %}}
The package automatically installs and enables the `ostserver` service. The service runs as the user who invoked `sudo apt install`, and the `/var/lib/osterix/media` directory is initialized.
{{% /notice %}}

{{% notice style="info" title="WebSocket port" %}}
`ostserver` listens for WebSocket connections on port **9624**.
{{% /notice %}}

---

## 6. Install the front-end: osterix

Download the latest release from the OST GitLab:

```bash
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
```

Install the package (nginx will be installed automatically):

```bash
sudo apt install -y ./osterix-front_latest.deb
```

The package automatically installs:
- Angular static files in `/var/www/osterix/`
- Nginx config in `/etc/nginx/sites-available/osterix`
- Symlink `/etc/nginx/sites-enabled/osterix`
- Reloads Nginx

---

## 7. Verify the installation

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

## 8. First start

From a browser on the local network: `http://<machine-ip>/`

1. Start the INDI server with your drivers
2. Open the OST interface → `http://<ip>/`

Modules are accessible from the main menu.

---

## Services summary

| Service | Role | Port |
|---|---|---|
| `ostserver` | C++/Qt back-end, astrophoto orchestration | 9624 (WebSocket) |
| `nginx` | Serves the Angular front-end | 80 |
| `indiserver` | INDI hardware driver layer | 7624 |

---

## Update

```bash
sudo apt update && sudo apt upgrade -y
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
sudo apt install -y ./osterix-front_latest.deb
sudo systemctl restart ostserver
```

`ostserver` updates via `apt upgrade` like any system package. Nginx reloads automatically via the osterix package postinst script.

---

## Uninstall

```bash
sudo apt remove ostserver osterix-front
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
