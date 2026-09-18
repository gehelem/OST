---
title: Instalación desde los paquetes .deb
weight: 10
description: "Procedimiento completo en Ubuntu 22.04 / 24.04 / 26.04 LTS a partir de los paquetes .deb de GitLab"
---

Requisitos previos: Ubuntu Server 22.04 (Jammy), 24.04 (Noble) o 26.04 (Resolute) limpio, acceso sudo, conexión a Internet.

---

## 1. Actualización del sistema

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl wget gnupg2 ca-certificates lsb-release apt-transport-https
```

---

## 2. Instalación de INDI

INDI es la capa de abstracción de hardware que controla cámaras, monturas, enfocadores y ruedas de filtros. `ostserver` depende de `libindi1`.

{{% notice style="note" title="¿Por qué no el PPA mutlaqja?" %}}
Desde agosto de 2026, el PPA `ppa:mutlaqja/ppa` solo publica para Ubuntu 26.04. Por ello, OST proporciona un conjunto fijo de paquetes INDI 2.2.4 — acompañados de `libxisf` y de `gsc` — en su propio registro, para las tres versiones LTS.
{{% /notice %}}

Descargue el archivo correspondiente a su versión de Ubuntu e instale su contenido:

```bash
# Elija según su versión:
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)
# INDI_TAR=indi-deps-resolute.tar # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt install -y ./indi-deps/*.deb
```

`apt` completa automáticamente las dependencias restantes (`libgsl`, `liberfa`, `libpugixml`…) desde el repositorio `universe` — asegúrese de que esté activado (ver §4).

El paquete `gsc` (Global Star Catalog, utilizado por el simulador CCD de INDI para producir un campo de estrellas realista) está incluido en el archivo — ningún paso manual.


---

## 3. Instalación de los índices de Astrometry.net

Necesarios para el plate solving (módulos Navigator y Polar).

```bash
sudo apt install -y astrometry.net
```

Los archivos de índice no están disponibles vía apt — descárguelos manualmente desde `http://data.astrometry.net/`.

{{% notice style="info" title="Elegir los índices correctos" %}}
Descargue únicamente los índices adecuados a su configuración. Como regla general: comience con los índices 4208–4210 (campos amplios) y añada 4206–4207 si tiene una distancia focal larga. Los índices son voluminosos — varios cientos de MB cada uno.
{{% /notice %}}

Descomente las líneas correspondientes a su configuración:

```bash
# --- Campos amplios > 5° (gran angular, focal corta) ---
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4214.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4213.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4212.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4211.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4210.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4209.fits
sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4208.fits

# --- Campos medios 1°–5° (focal larga) — 12 archivos por índice ---
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-00.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-01.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-02.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-03.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-04.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-05.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-06.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-07.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-08.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-09.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-10.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4207-11.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-00.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-01.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-02.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-03.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-04.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-05.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-06.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-07.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-08.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-09.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-10.fits
#sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-4206-11.fits
```

Para descargar todos los índices de una vez (varios GB):

```bash
for idx in 4208 4209 4210 4211 4212 4213 4214; do
    sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}.fits
done
for idx in 4206 4207; do
    for i in $(seq -w 0 11); do
        sudo wget -P /usr/share/astrometry http://data.astrometry.net/4200/index-${idx}-${i}.fits
    done
done
```

---

## 4. Dependencias del sistema

El paquete `ostserver` requiere varias bibliotecas Qt6, GSL, cfitsio, libnova y StellarSolver.
`apt` las resolverá automáticamente, pero asegúrese de que el repositorio `universe` esté activado:

```bash
sudo add-apt-repository universe -y
sudo apt update
```

`libstellarsolver` está incluido directamente en el paquete `ostserver` — aún no se publica ninguna versión Qt6 de forma independiente.

---

## 5. Instalación del back-end: ostserver

Descargue la última release desde el GitLab de OST, eligiendo el paquete correspondiente a su versión de Ubuntu:

```bash
DEB=ostserver_latest_2404_amd64.deb      # Ubuntu 24.04 (Noble)
# DEB=ostserver_latest_2204_amd64.deb    # Ubuntu 22.04 (Jammy)
# DEB=ostserver_latest_2604_amd64.deb    # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
sudo apt install -y "./${DEB}"
```

`apt` resuelve automáticamente todas las dependencias.

El paquete instala:
- `/usr/bin/ostserver` — el binario principal
- `/usr/lib/libost*.so` — los módulos (focus, guider, sequencer, navigator, planner, polar, inspector, indipanel, allsky)

{{% notice style="info" title="Servicio systemd incluido" %}}
El paquete instala y activa automáticamente el servicio `ostserver`. El servicio se ejecuta bajo el usuario que lanzó `sudo apt install`, y el directorio `/var/lib/osterix/media` se inicializa.
{{% /notice %}}

{{% notice style="info" title="Puerto WebSocket" %}}
`ostserver` escucha en WebSocket en el puerto **9624**.
{{% /notice %}}

---

## 6. Instalación del front-end: osterix

Descargue la última release desde el GitLab de OST:

```bash
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
```

Instale el paquete (nginx se instalará automáticamente):

```bash
sudo apt install -y ./osterix-front_latest.deb
```

El paquete instala automáticamente:
- Los archivos estáticos de Angular en `/var/www/osterix/`
- La configuración de Nginx en `/etc/nginx/sites-available/osterix`
- El enlace simbólico `/etc/nginx/sites-enabled/osterix`
- Recarga Nginx

Configuración de Nginx depositada por el paquete:

```nginx
server {
    listen 80;
    root /var/www/osterix;
    index index.html;
    server_name _;

    location / {
        try_files $uri $uri/ /index.html;
    }

    location /ostmedia/ {
        alias /var/lib/osterix/media/;
        autoindex on;
    }
}
```

---

## 7. Verificación de la instalación

```bash
# Estado de los servicios
sudo systemctl status ostserver
sudo systemctl status nginx

# Puertos en escucha
ss -tlnp | grep -E '80|9624|7624'

# Logs de ostserver en tiempo real
sudo journalctl -u ostserver -f

# Probar la configuración de Nginx
sudo nginx -t
```

---

## 8. Primer arranque

Desde un navegador en la red local: `http://<ip-de-la-máquina>/`

1. Iniciar el servidor INDI con sus controladores
2. Abrir la interfaz OST → `http://<ip>/`

Los módulos son accesibles desde el menú principal.

---

## Resumen de los servicios

| Servicio | Función | Puerto |
|---|---|---|
| `ostserver` | Back-end C++/Qt, orquestación astrofoto | 9624 (WebSocket) |
| `nginx` | Sirve el front Angular | 80 |
| `indiserver` | Capa de controladores de hardware INDI | 7624 |

---

## Actualización

```bash
DEB=ostserver_latest_2404_amd64.deb      # Ubuntu 24.04 (Noble)
# DEB=ostserver_latest_2204_amd64.deb    # Ubuntu 22.04 (Jammy)
# DEB=ostserver_latest_2604_amd64.deb    # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB}"
wget https://gitlab.ostserver.fr/api/v4/projects/6/packages/generic/latest/latest/osterix-front_latest.deb
sudo apt install -y "./${DEB}" ./osterix-front_latest.deb
sudo systemctl restart ostserver
```

Nginx se recarga automáticamente vía el postinst del paquete osterix.

---

## Desinstalación

```bash
sudo apt remove ostserver osterix-front
sudo systemctl disable --now ostserver
sudo rm /etc/systemd/system/ostserver.service
sudo systemctl daemon-reload
```
