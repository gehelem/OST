---
title: Quick install (Ubuntu Resolute)
weight: 5
description: "The fastest way to get OST running: one fresh Ubuntu 26.04 machine, one script"
---

{{% notice style="tip" title="Best for beginners" %}}
Never used a Linux command line before? This is the page for you. Just
follow the three steps below, in order.
{{% /notice %}}

This single script installs everything needed on one machine: the OST
backend, the web interface, nginx, INDI, `astrometry.net` and the
documentation you are reading right now.

{{% notice style="note" title="Using Arch Linux instead?" %}}
This page is for Ubuntu specifically. On Arch Linux, follow
[Quick install (Arch Linux)](../quick-arch/) instead.
{{% /notice %}}

---

## 1. Install a fresh Ubuntu 26.04

Install **Ubuntu Server 26.04 LTS ("Resolute")** on the machine that
will run OST — a Raspberry Pi, a mini-PC, or a virtual machine. Use
the standard Ubuntu installer; no special option is needed.

{{% notice style="note" title="Why 26.04 specifically?" %}}
This script downloads the package built for Ubuntu 26.04. The other
supported versions (22.04, 24.04) need a different, more manual
procedure — see [Full installation procedure](../fresh/) instead.
{{% /notice %}}

Once Ubuntu is installed, open a terminal (directly on the machine,
or over SSH from another computer).

---

## 2. Download the install script

Copy the command below, paste it into the terminal, and press Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete.sh
```

Nothing happens on screen if it works — that is normal. A new file,
`install-latest-complete.sh`, now exists in your current folder.

---

## 3. Run the script

```bash
chmod +x install-latest-complete.sh
./install-latest-complete.sh
```

The script then does everything by itself. A few things to expect:

- It will ask for **your password** a few times (`sudo`) — this is
  normal, it needs permission to install system packages.
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
layout, nginx as the web server, Ubuntu 26.04 only). If you need
something different, follow [Full installation procedure](../fresh/)
instead — same result, every step explained and adjustable.
{{% /notice %}}
