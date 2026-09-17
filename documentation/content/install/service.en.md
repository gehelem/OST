---
title: Managing the ostserver service
weight: 30
description: "Start, stop, and customize the startup options of the ostserver service"
---

`ostserver` runs in the background as a **systemd service**. Systemd is Linux's service manager: it automatically starts `ostserver` when the machine boots, restarts it if it crashes, and lets you control it with a few simple commands.

---

## Start, stop, restart

```bash
# Start the service
sudo systemctl start ostserver

# Stop the service
sudo systemctl stop ostserver

# Restart the service (useful after changing options)
sudo systemctl restart ostserver

# Check the service status
sudo systemctl status ostserver
```

---

## Enable / disable automatic startup

By default, the service is **enabled**: it starts automatically every time the machine boots.

```bash
# Disable automatic startup
sudo systemctl disable ostserver

# Re-enable automatic startup
sudo systemctl enable ostserver
```

---

## View logs

```bash
# Show recent logs
sudo journalctl -u ostserver

# Follow logs in real time
sudo journalctl -u ostserver -f

# Limit to the last 100 lines
sudo journalctl -u ostserver -n 100
```

---

## Customize startup options

During installation, a configuration file is created in your home directory:

```
~/.ost/ostserver.parms
```

This file contains **one argument per line**. Lines starting with `#` are comments and are ignored. Spaces in values are supported natively — no quoting is needed.

To edit it:

```bash
nano ~/.ost/ostserver.parms
```

Example content:

```
# Web server media folder (must be writable)
--webroot=/var/lib/osterix/media

# Start embedded INDI server (Y/N)
--indiserver=Y

# Log level: 0=debug, 1=info, 2=warning, 3=error, 4=critical
--loglevel=1

# Security level: 0=disabled, 1=read only, 2=user grants
--grant=0

# Frontend banner text (spaces are allowed, no quoting needed)
--banner=My headless observatory
```

After any change, restart the service for it to take effect:

```bash
sudo systemctl restart ostserver
```

{{% notice style="info" title="File preserved on upgrades" %}}
The `ostserver.parms` file is **never overwritten** when the package is upgraded. Your customizations are always kept.
{{% /notice %}}

---

## Available options reference

| Option | Default | Description |
|---|---|---|
| `--webroot=<path>` | `/var/lib/osterix/media` | Media folder served by nginx |
| `--indiserver=Y\|N` | `N` | Start the embedded INDI server |
| `--loglevel=<0-4>` | `1` | Log verbosity: 0=debug, 1=info, 2=warning, 3=error, 4=critical |
| `--logfile=<path>` | *(timestamped in `~/.ost/`)* | Log file path |
| `--grant=<0\|1\|2>` | — | Security: 0=disabled, 1=read only, 2=user grants |
| `--lng=<fr\|en\|...>` | `fr` | Interface language (fr, en, de, it, es) |
| `--banner=<text>` | `Observatoire Sans tête` | Banner displayed in the frontend |
| `--configuration=<name>` | `default` | Configuration name to load at startup |
| `--libpath=<path>` | *(system path)* | Directory for `.so` module files |
| `--dbpath=<path>` | *(working directory)* | Database location |
| `--systemwatchinterval=<s>` | `10` | System resource watch interval in seconds (0 to disable) |
| `--minfree=<0-99>` | `10` | Minimum free disk space in % before skipping image write |
| `--ssl=Y\|N` | `N` | Enable SSL for the WebSocket server |
| `--sslcert=<path>` | `/etc/ost/server.crt` | SSL certificate file |
| `--sslkey=<path>` | `/etc/ost/server.key` | SSL private key file |
| `--setadminpassword=<pwd>` | — | Set the administrator password (remove this line after use) |
