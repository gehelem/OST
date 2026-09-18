---
title: Navigator
weight: 40
description: Búsqueda de objetivos y puesta en estación precisa mediante plate solving
---

## Función

El módulo Navigator permite buscar un objeto en los catálogos astronómicos, apuntar la montura hacia ese objeto, y luego centrar con precisión el campo en bucle mediante plate solving (resolución astrométrica). También puede enviar el objetivo seleccionado al módulo Planner.

![Captura de pantalla del módulo Navigator](/images/modules/navigator.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| Cámara | Adquisición de las imágenes para el plate solving |
| Montura | Recepción de las órdenes de desplazamiento |
| GPS | Suministro de las coordenadas geográficas y de la hora |

## Parámetros

### Adquisición

| Parámetro | Descripción |
|---|---|
| **Exposición** | Duración de exposición en segundos |
| **Ganancia** | Ganancia de la cámara |
| **Offset** | Offset de la cámara |
| **Focal** | Distancia focal óptica en mm — usada para calcular la escala de imagen del solver |

### Centrado

| Parámetro | Descripción |
|---|---|
| **Iteraciones máx** | Número máximo de ciclos slew → solve → correct antes de abandonar |
| **Tolerancia (arcsec)** | Distancia angular aceptable entre el centro resuelto y el objetivo |
| **Sincronizar la montura** | Si está activado, sincroniza las coordenadas de la montura con la solución tras un centrado exitoso |

### Módulos esclavos

| Parámetro | Descripción |
|---|---|
| **Instancia del módulo Planner** | Nombre de la instancia del módulo Planner a usar para la acción *Añadir a la planificación* |

## Funcionamiento

### Búsqueda de objetivos

La búsqueda se realiza introduciendo un nombre o código en el campo **Buscar un objeto**. Los catálogos disponibles son:

| Catálogo | Contenido |
|---|---|
| Messier | Objetos del catálogo Messier |
| NGC | New General Catalogue |
| LDN | Lynds' Catalogue of Dark Nebulae |
| SH2 | Sharpless catalogue (nebulosas H II) |
| IC | Index Catalogue |
| Estrellas | Estrellas con nombre |

Los resultados se muestran en la cuadrícula **Resultados** con: catálogo, código, RA, DEC, magnitud, diámetro, nombre y alias. Un clic en una fila selecciona el objetivo.

### Conversión J2000 → época actual

En cuanto se selecciona un objetivo, sus coordenadas J2000 se convierten a las coordenadas de la época actual (teniendo en cuenta la precesión) y se muestran en **Selección convertida**.

### Bucle de centrado

La acción **Goto objetivo** desencadena la siguiente secuencia:

1. La montura apunta hacia las coordenadas convertidas a la época actual
2. La cámara adquiere una imagen que es resuelta por el solver astrométrico (StellarSolver / índices astrometry.net)
3. Se calcula la distancia angular entre el centro resuelto y el objetivo
4. Si la distancia es inferior a la **Tolerancia**, el centrado se declara exitoso
5. En caso contrario, se calcula y aplica un offset correctivo a la montura; el ciclo se reinicia

Este proceso se repite hasta el éxito o hasta alcanzar el número máximo de iteraciones.

{{% notice style="tip" title="Sincronización de la montura" %}}
Si *Sincronizar la montura* está activado, el módulo realiza un SYNC INDI tras el centrado exitoso, lo que mejora la precisión de los apuntados siguientes.
{{% /notice %}}

## Valores mostrados

### Posición de la montura

| Valor | Descripción |
|---|---|
| **RA** | Ascensión recta actual de la montura |
| **DEC** | Declinación actual de la montura |

### Posición GPS

| Valor | Descripción |
|---|---|
| **Altitud** | Altitud del lugar de observación (m) |
| **Latitud** | Latitud del lugar de observación (°) |
| **Longitud** | Longitud del lugar de observación (°) |

### Fecha y hora GPS

| Valor | Descripción |
|---|---|
| **Desfase UTC** | Zona horaria |
| **Fecha** | Fecha actual |
| **Hora** | Hora actual |

## Acciones

| Acción | Descripción |
|---|---|
| **Goto objetivo** | Apunta la montura hacia el objetivo seleccionado y lanza el bucle de centrado |
| **Abandonar** | Interrumpe inmediatamente el goto o el centrado en curso |
| **Añadir a la planificación** | Envía el objetivo actual al módulo Planner |
