---
title: Instalación rápida (Ubuntu Resolute)
weight: 5
description: "La forma más rápida de poner en marcha OST: una máquina con Ubuntu 26.04 recién instalado, un script"
---

{{% notice style="tip" title="Ideal para principiantes" %}}
¿Nunca ha usado antes una línea de comandos de Linux? Esta es la página para usted. Simplemente
siga los tres pasos siguientes, en orden.
{{% /notice %}}

Este único script instala todo lo necesario en una sola máquina: el back-end de
OST, la interfaz web, nginx, INDI, `astrometry.net` y la
documentación que está leyendo ahora mismo.

{{% notice style="note" title="¿Usa Arch Linux en su lugar?" %}}
Esta página es específica para Ubuntu. En Arch Linux, siga en su lugar
[Instalación rápida (Arch Linux)](../quick-arch/).
{{% /notice %}}

---

## 1. Instale un Ubuntu 26.04 recién instalado

Instale **Ubuntu Server 26.04 LTS ("Resolute")** en la máquina que
va a ejecutar OST — una Raspberry Pi, un mini-PC o una máquina virtual. Use
el instalador estándar de Ubuntu; no se necesita ninguna opción especial.

{{% notice style="note" title="¿Por qué específicamente 26.04?" %}}
Este script descarga el paquete compilado para Ubuntu 26.04. Las otras
versiones compatibles (22.04, 24.04) requieren un procedimiento diferente,
más manual — consulte en su lugar [Procedimiento completo de instalación](../fresh/).
{{% /notice %}}

Una vez instalado Ubuntu, abra una terminal (directamente en la máquina,
o por SSH desde otro ordenador).

---

## 2. Descargue el script de instalación

Copie el siguiente comando, péguelo en la terminal y pulse Enter:

```bash
wget https://gitlab.ostserver.fr/ost/ostserver/-/raw/main/install-latest-complete.sh
```

No ocurre nada en pantalla si funciona correctamente — es normal. Un nuevo archivo,
`install-latest-complete.sh`, existe ahora en su carpeta actual.

---

## 3. Ejecute el script

```bash
chmod +x install-latest-complete.sh
./install-latest-complete.sh
```

El script se encarga entonces de todo por sí mismo. Algunas cosas que debe esperar:

- Le pedirá **su contraseña** varias veces (`sudo`) — esto es
  normal, necesita permisos para instalar paquetes del sistema.
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
estándar, nginx como servidor web, solo Ubuntu 26.04). Si necesita
algo diferente, siga en su lugar [Procedimiento completo de instalación](../fresh/)
— mismo resultado, con cada paso explicado y ajustable.
{{% /notice %}}
