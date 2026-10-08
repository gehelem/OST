---
title: Quick install (Arch Linux)
weight: 6
description: "The fastest way to get OST running on Arch Linux: one script"
---

{{% notice style="tip" title="Best for beginners" %}}
Never used a Linux command line before? This is the page for you. Just
follow the three steps below, in order.
{{% /notice %}}

This single script installs everything needed on one machine: the OST
backend, the web interface, nginx, INDI, `astrometry.net` and the
documentation you are reading right now.

{{% notice style="note" title="Using Ubuntu instead?" %}}
This page is for Arch Linux specifically. On Ubuntu, follow
[Quick install (Ubuntu Resolute)](../quick/) instead.
{{% /notice %}}

---

## 1. Install a fresh Arch Linux

Install **Arch Linux** on the machine that will run OST — a
Raspberry Pi, a mini-PC, or a virtual machine. Use the standard Arch
installer; no special option is needed.

Once Arch is installed, open a terminal (directly on the machine, or
over SSH from another computer).

---

## 2. Download the install script

A minimal Arch install does not include `wget` - install it first:

```bash
sudo pacman -S --needed wget
```

Then copy the command below, paste it into the terminal, and press Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-arch.sh
```

Nothing happens on screen if it works — that is normal. A new file,
`install-latest-complete-arch.sh`, now exists in your current folder.

---

## 3. Run the script

```bash
chmod +x install-latest-complete-arch.sh
./install-latest-complete-arch.sh
```

The script then does everything by itself. A few things to expect:

- It will ask for **your password** a few times (`sudo`) — this is
  normal, it needs permission to install system packages.
- It builds two packages from AUR (`astrometry.net` and `gsc`) -
  this compiles real code on your machine and can take a few minutes.
  It is normal to see pages of compiler output scroll by.
- `gsc` is known to currently fail to build (an old, unmaintained AUR
  package - not an OST problem). The script prints a warning and
  carries on without it: `gsc` only feeds INDI's CCD *simulator* with
  a realistic star field, it is never needed for real hardware.
- A blue screen may appear asking which `astrometry.net` index files
  to download in addition to the default ones. Use the **arrow
  keys** to move, **Space** to check/uncheck, **Tab** then **Enter**
  to confirm. If you are not sure, leave everything unchecked and
  press Enter — you can always add them later.

At the very end, it prints two addresses, for example:

```
Web UI: http://192.168.1.42/
Documentation: http://192.168.1.42/documentation/
```

---

## Done

Open the first address (`Web UI: ...`) in a web browser, from any
device on the same local network as the machine you just installed
OST on. You should see the OST interface.

{{% notice style="info" title="Need more control?" %}}
This script makes choices for you (SSL disabled, standard folder
layout, nginx as the web server). If you need something different,
install the `ostserver`/`ostserver-complete` package by hand instead
(from AUR, or the prebuilt `.pkg.tar.zst` on the releases page) and
wire up the rest yourself — the script's own steps double as a guide
for what to do manually.
{{% /notice %}}
