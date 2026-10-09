---
title: Instalación rápida (Arch Linux)
weight: 6
description: "La forma más rápida de poner en marcha OST en Arch Linux: un script"
---

{{% notice style="tip" title="Ideal para principiantes" %}}
¿Nunca ha usado antes una línea de comandos de Linux? Esta es la página para usted. Simplemente
siga los tres pasos siguientes, en orden.
{{% /notice %}}

Este único script instala todo lo necesario en una sola máquina: el back-end de
OST, la interfaz web, nginx, INDI, `astrometry.net` y la
documentación que está leyendo ahora mismo.

{{% notice style="note" title="¿Usa Ubuntu en su lugar?" %}}
Esta página es específica para Arch Linux. En Ubuntu, siga en su lugar
[Instalación rápida (Ubuntu Resolute)](../quick/).
{{% /notice %}}

---

## 1. Instale un Arch Linux recién instalado

Instale **Arch Linux** en la máquina que va a ejecutar OST — una
Raspberry Pi, un mini-PC o una máquina virtual. Use
el instalador estándar de Arch; no se necesita ninguna opción especial.

Una vez instalado Arch, abra una terminal (directamente en la máquina,
o por SSH desde otro ordenador).

---

## 2. Descargue el script de instalación

Una instalación mínima de Arch no incluye `wget` - instálelo primero:

```bash
sudo pacman -S --needed wget
```

A continuación, copie el siguiente comando, péguelo en la terminal y pulse Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete-arch.sh
```

No ocurre nada en pantalla si funciona correctamente — es normal. Un nuevo archivo,
`install-latest-complete-arch.sh`, existe ahora en su carpeta actual.

---

## 3. Ejecute el script

```bash
chmod +x install-latest-complete-arch.sh
./install-latest-complete-arch.sh
```

El script se encarga entonces de todo por sí mismo. Algunas cosas que debe esperar:

- Le pedirá **su contraseña** varias veces (`sudo`) — esto es
  normal, necesita permisos para instalar paquetes del sistema.
- Compila `astrometry.net` desde AUR - esto compila código real en su
  máquina y puede tardar varios minutos. Es normal ver pasar páginas
  de salida del compilador.
- Instala `gsc` (datos de campo de estrellas realista solo para el
  *simulador* de CCD de INDI, nunca necesario para hardware real)
  desde un paquete precompilado propio - el paquete de AUR ya no
  compila en Arch actual, sin relación con OST.
- Puede aparecer una pantalla azul preguntando qué archivos de índice de
  `astrometry.net` descargar además de los predeterminados. Use las **teclas
  de flecha** para moverse, **Espacio** para marcar/desmarcar, **Tab** y luego **Enter**
  para confirmar. Si no está seguro, deje todo sin marcar y
  pulse Enter — siempre podrá añadirlos más tarde.

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
lugar el paquete `ostserver`/`ostserver-complete` a mano (desde AUR, o el
`.pkg.tar.zst` precompilado en la página de releases) y configure el resto
usted mismo — los propios pasos del script sirven también de guía para lo que
hay que hacer manualmente.
{{% /notice %}}
