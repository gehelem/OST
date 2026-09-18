---
title: Presentación
weight : 10
---

## ¿Por qué "Observatoire Sans Tête" (Observatorio Sin Cabeza)?

Este nombre viene de una broma con mi amigo Bob, que consiste en traducir palabra por palabra "Headless Observatory".
Este término refleja bien el objetivo buscado: un servidor autónomo que se puede controlar a distancia.

## ¿Qué es OST?

OST es una plataforma de astrofotografía autónoma. Se ejecuta en segundo plano en una máquina dedicada — Raspberry Pi, mini-PC o máquina virtual — y se utiliza desde cualquier navegador web en la red local, sin pantalla ni teclado conectados a la máquina de control.

El objetivo es poder pilotar una sesión completa de observación desde un sofá, otro edificio, o a distancia vía VPN: puesta en estación, apuntado, enfoque, guiado, adquisición y documentación de la noche.

## Arquitectura

OST se basa en una arquitectura cliente-servidor:

| Componente | Función |
|---|---|
| **Servidor OST** | Aplicación C++/Qt que se ejecuta en segundo plano, pilota los dispositivos mediante el protocolo INDI |
| **Interfaz web** | Cliente Angular servido por Nginx, se comunica con el servidor vía WebSocket |
| **Servidor INDI** | Capa de abstracción de hardware — controladores para cámaras, monturas, enfocadores, ruedas de filtros, etc. |

La comunicación entre el cliente y el servidor se basa enteramente en mensajes JSON intercambiados vía WebSocket, lo que permite controlar OST desde cualquier dispositivo con un navegador moderno.

## Módulos

OST está organizado en módulos independientes, cada uno dedicado a una función específica:

| Módulo | Función |
|---|---|
| **Secuenciador** | Adquisición automatizada: secuencias de exposiciones por tipo, filtro, ganancia y duración |
| **Guiado** | Autoguiado vía PHD2, control de la agitación y de las correcciones |
| **Navegador** | Resolución de campo (plate solving) y centrado automático sobre un objetivo |
| **Planificador** | Orquestación de una lista de objetos: centrado y luego adquisición para cada uno |
| **Polar** | Alineación polar mediante el método de las 3 imágenes |
| **Enfoque** | Autoenfoque mediante curva en V con minimización del HFR |
| **Inspector** | Análisis de la calidad óptica: mapa HFR, mapa de aberraciones, mosaico de esquinas |
| **IndiPanel** | Panel de control bruto de todos los dispositivos INDI conectados |
| **Allsky** | Cámara gran angular: keograma, stack, timelapse y superposición meteorológica |

## Inspiraciones

En cuanto a su forma, OST se inspira en numerosos otros proyectos opensource:
- https://github.com/GuLinux/AstroPhoto-Plus
- https://github.com/pludov/mobindi
- https://www.indigo-astronomy.org/for-users.html

Por supuesto, la inspiración también viene de soluciones comerciales:
- ASIAIR
- Stellarmate

## Tecnologías utilizadas

| Ámbito | Tecnología |
|---|---|
| Servidor | C++17, Qt6, libnova, cfitsio, libindi, StellarSolver |
| Interfaz web | Angular, TypeScript |
| Servidor web | Nginx |
| Protocolo | INDI, WebSocket, JSON |
| Vídeo | ffmpeg (timelapse allsky) |
| Empaquetado | PPA Ubuntu (Launchpad) |
