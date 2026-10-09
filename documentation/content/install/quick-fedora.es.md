---
title: Instalación rápida (Fedora)
weight: 7
description: "La forma más rápida de poner en marcha OST en Fedora: un script"
---

{{% notice style="tip" title="Ideal para principiantes" %}}
¿Nunca ha usado antes una línea de comandos de Linux? Esta es la página para usted. Simplemente
siga los tres pasos siguientes, en orden.
{{% /notice %}}

Este único script instala todo lo necesario en una sola máquina: el back-end de
OST, la interfaz web, nginx, INDI, `astrometry.net` y la
documentación que está leyendo ahora mismo.

{{% notice style="note" title="¿Usa Ubuntu o Arch en su lugar?" %}}
Esta página es específica para Fedora. En su lugar, siga
[Instalación rápida (Ubuntu Resolute)](../quick/) o
[Instalación rápida (Arch Linux)](../quick-arch/).
{{% /notice %}}

---

## 1. Instale un Fedora recién instalado

Instale **Fedora Server** (o Workstation) en la máquina que va a
ejecutar OST — una Raspberry Pi, un mini-PC o una máquina virtual. Use
el instalador estándar de Fedora; no se necesita ninguna opción especial.

Una vez instalado Fedora, abra una terminal (directamente en la máquina,
o por SSH desde otro ordenador).

---

## 2. Descargue el script de instalación

Copie el siguiente comando, péguelo en la terminal y pulse Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-fedora.sh
```

No ocurre nada en pantalla si funciona correctamente — es normal. Un nuevo archivo,
`install-latest-complete-fedora.sh`, existe ahora en su carpeta actual.

---

## 3. Ejecute el script

```bash
chmod +x install-latest-complete-fedora.sh
./install-latest-complete-fedora.sh
```

El script se encarga entonces de todo por sí mismo. Algunas cosas que debe esperar:

- Le pedirá **su contraseña** varias veces (`sudo`) — esto es
  normal, necesita permisos para instalar paquetes del sistema.
- Abre los puertos de la interfaz web (80) y del WebSocket (9624) en
  `firewalld`, que está activado por defecto en Fedora (a diferencia
  de Ubuntu/Arch) y que de otro modo bloquearía ambos desde cualquier
  otra máquina de su red.
- Elimina la página por defecto que trae nginx en Fedora (integrada en
  el propio `/etc/nginx/nginx.conf`) - de lo contrario se impone
  silenciosamente sobre la propia página de OST en el puerto 80.
- Instala `gsc` (datos de campo de estrellas realista solo para el
  *simulador* de CCD de INDI, nunca necesario para hardware real)
  desde un paquete precompilado propio - no existe ningún paquete
  oficial de Fedora para ello.
- Puede aparecer una pantalla azul preguntando qué archivos de índice de
  `astrometry.net` descargar además de los predeterminados. Use las **teclas
  de flecha** para moverse, **Espacio** para marcar/desmarcar, **Tab** y luego **Enter**
  para confirmar. Si no está seguro, deje todo sin marcar y
  pulse Enter — siempre podrá añadirlos más tarde.
- Muestra mensajes de progreso mientras descarga e instala. Esto puede
  tardar unos minutos según su conexión a Internet.

Al final del todo, muestra dos direcciones, por ejemplo:

```
Web UI: http://192.168.1.42/
Documentation: http://192.168.1.42/documentation/
```

---

## Listo

Abra la primera dirección (`Web UI: ...`) en un navegador web, desde cualquier
dispositivo de la misma red local que la máquina en la que acaba de instalar
OST. Debería ver la interfaz de OST.

{{% notice style="info" title="¿Necesita más control?" %}}
Este script toma decisiones por usted (SSL desactivado, estructura de carpetas
estándar, nginx como servidor web). Si necesita algo diferente, instale en su
lugar el paquete `ostserver`/`ostserver-complete` a mano (el `.rpm` precompilado
en la página de releases) y configure el resto usted mismo — los propios pasos
del script sirven también de guía para lo que hay que hacer manualmente.
{{% /notice %}}
