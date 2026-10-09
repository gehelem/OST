---
title: Quick install (Fedora)
weight: 7
description: "The fastest way to get OST running on Fedora: one script"
---

{{% notice style="tip" title="Best for beginners" %}}
Never used a Linux command line before? This is the page for you. Just
follow the three steps below, in order.
{{% /notice %}}

This single script installs everything needed on one machine: the OST
backend, the web interface, nginx, INDI, `astrometry.net` and the
documentation you are reading right now.

{{% notice style="note" title="Using Ubuntu or Arch instead?" %}}
This page is for Fedora specifically. See
[Quick install (Ubuntu Resolute)](../quick/) or
[Quick install (Arch Linux)](../quick-arch/) instead.
{{% /notice %}}

---

## 1. Install a fresh Fedora

Install **Fedora Server** (or Workstation) on the machine that will
run OST — a Raspberry Pi, a mini-PC, or a virtual machine. Use the
standard Fedora installer; no special option is needed.

Once Fedora is installed, open a terminal (directly on the machine, or
over SSH from another computer).

---

## 2. Download the install script

Copy the command below, paste it into the terminal, and press Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-fedora.sh
```

Nothing happens on screen if it works — that is normal. A new file,
`install-latest-complete-fedora.sh`, now exists in your current folder.

---

## 3. Run the script

```bash
chmod +x install-latest-complete-fedora.sh
./install-latest-complete-fedora.sh
```

The script then does everything by itself. A few things to expect:

- It will ask for **your password** a few times (`sudo`) — this is
  normal, it needs permission to install system packages.
- It opens the web UI (80) and WebSocket (9624) ports in `firewalld`,
  which is on by default on Fedora (unlike Ubuntu/Arch) and would
  otherwise block both from any other machine on your network.
- It removes the stock default page nginx ships with on Fedora (built
  into `/etc/nginx/nginx.conf` itself) - otherwise it silently wins
  over OST's own page on port 80.
- It installs `gsc` (realistic star field data for INDI's CCD
  *simulator* only, never needed for real hardware) from a prebuilt
  package of our own - no official Fedora package exists for it.
- A blue screen may appear asking which `astrometry.net` index files
  to download in addition to the default ones. Use the **arrow
  keys** to move, **Space** to check/uncheck, **Tab** then **Enter**
  to confirm. If you are not sure, leave everything unchecked and
  press Enter — you can always add them later.
- It prints progress messages as it downloads and installs. This can
  take a few minutes depending on your Internet connection.

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
(the prebuilt `.rpm` on the releases page) and wire up the rest
yourself — the script's own steps double as a guide for what to do
manually.
{{% /notice %}}
