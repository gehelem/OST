---
title: Instalación desde el PPA de Ubuntu (Launchpad)
weight: 20
description: "Procedimiento completo en Ubuntu 26.04 LTS usando el repositorio PPA oficial"
---

Requisitos previos: Ubuntu Server 26.04 (Resolute) limpio, acceso sudo, conexión a Internet.

Este método utiliza el PPA oficial de Launchpad — `ostserver-complete` se instala vía `apt` y se actualiza automáticamente con `apt upgrade`.

{{% notice style="note" title="¿22.04 / 24.04?" %}}
Actualmente el PPA solo publica para Ubuntu 26.04 — en 22.04 (Jammy) o 24.04 (Noble), utilice en su lugar el [procedimiento manual con paquetes .deb]({{% relref "fresh.es.md" %}}).
{{% /notice %}}

---

## 1. Actualización del sistema

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Instalación de INDI

INDI es la capa de abstracción de hardware que controla cámaras, monturas, enfocadores y ruedas de filtros. `ostserver` depende de `libindi1`, y la mayoría de las cámaras (ZWO, QHY, PlayerOne y otras) necesitan su controlador de `indi-3rdparty-drivers`.

```bash
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo add-apt-repository -y universe
sudo apt update
sudo apt install -y libindi1 indi-bin gsc indi-3rdparty-libs indi-3rdparty-drivers
```

---

## 3. Instalación de los índices de Astrometry.net

Necesarios para el plate solving (módulos Navigator y Polar).

```bash
sudo apt install -y astrometry.net
```

Los archivos de índice no están disponibles vía apt — descárguelos manualmente desde `http://data.astrometry.net/`.

{{% notice style="info" title="Elegir los índices correctos" %}}
Descargue únicamente los índices adecuados a su configuración. Como regla general: los índices 4208–4210 cubren campos amplios. Añada 4206–4207 para distancias focales más largas. Cada índice ocupa varios cientos de MB.
{{% /notice %}}

Descomente las líneas correspondientes a su configuración:

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

Para descargar todos los índices de una vez (varios GB):

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

`-nc` (no-clobber) omite un archivo ya descargado — puede volver a ejecutar esta sección sin riesgo para completar un conjunto parcial.

---

## 4. Dependencias del sistema

El paquete `ostserver-complete` requiere varias bibliotecas Qt6, GSL, cfitsio, libnova y StellarSolver, además de nginx.
`apt` las resolverá automáticamente, pero asegúrese de que el repositorio `universe` esté activado:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` está incluido directamente en el paquete — aún no se publica ninguna versión Qt6 de forma independiente.

---

## 5. Instalación de ostserver-complete

Añada el PPA oficial de OST e instale `ostserver-complete` — el back-end, todos los módulos, la interfaz web y la configuración de nginx que los conecta, todo en un único paquete:

```bash
sudo add-apt-repository ppa:gehelem/ostserver -y
sudo apt update
sudo apt install -y ostserver-complete
```

`apt` resuelve automáticamente todas las dependencias, incluido nginx.

El paquete instala:
- `/usr/bin/ostserver` — el binario principal
- `/usr/lib/libost*.so` — los módulos (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)
- los archivos estáticos de la interfaz web, con un sitio nginx ya configurado para servirlos

{{% notice style="info" title="Servicio systemd incluido" %}}
El paquete instala y activa automáticamente el servicio `ostserver`. El servicio se ejecuta bajo el usuario que lanzó `sudo apt install`, y el directorio `/var/lib/osterix/media` se inicializa.
{{% /notice %}}

{{% notice style="info" title="Puerto WebSocket" %}}
`ostserver` escucha en WebSocket en el puerto **9624**.
{{% /notice %}}

---

## 6. Verificación de la instalación

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

## 7. Primer arranque

Desde un navegador en la red local: `http://<machine-ip>/`

1. Inicie el servidor INDI con sus controladores
2. Abra la interfaz OST → `http://<ip>/`

Los módulos son accesibles desde el menú principal.

---

## Resumen de los servicios

| Servicio | Función | Puerto |
|---|---|---|
| `ostserver` | Back-end C++/Qt, orquestación astrofoto | 9624 (WebSocket) |
| `nginx` | Sirve la interfaz web | 80 |
| `indiserver` | Capa de controladores de hardware INDI | 7624 |

---

## Actualización

```bash
sudo apt update && sudo apt upgrade -y
```

`ostserver-complete` se actualiza vía `apt upgrade` como cualquier paquete del sistema — nginx y los archivos estáticos de la interfaz web se actualizan junto con él.

---

## Desinstalación

```bash
sudo apt remove ostserver-complete
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
