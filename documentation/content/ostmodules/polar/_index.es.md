---
title: Polar
weight: 60
description: Asistente de puesta en estación polar mediante plate solving
---

## Función

El módulo Polar asiste en la puesta en estación polar de la montura. Adquiere tres imágenes del cielo desplazando la montura en RA entre cada toma, resuelve astrométricamente cada imagen, y luego calcula el error de azimut y de altitud del eje polar. Un bucle de corrección continua permite después guiar al usuario mientras ajusta mecánicamente la montura.

![Captura de pantalla del módulo Polar](/images/modules/polar.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| Cámara | Adquisición de las imágenes para el plate solving |
| Montura | Desplazamiento en RA y lectura de las coordenadas |
| GPS | Posición geográfica del observador |

## Parámetros

| Parámetro | Descripción |
|---|---|
| **Exposición** | Duración de exposición en segundos |
| **Ganancia** | Ganancia de la cámara |
| **Offset** | Offset de la cámara |
| **Focal** | Distancia focal óptica en mm — usada para definir la escala de imagen del solver |

## Algoritmo

### Fase 1 — Calibración (3 imágenes)

El módulo adquiere tres imágenes realizando un desplazamiento de 30 minutos de ángulo (0,5 h) en RA entre cada toma:

1. **Imagen 0** — posición inicial
2. Desplazamiento de ±0,5 h en RA (sentido adaptado al lado del pie)
3. **Imagen 1** — nueva posición
4. Nuevo desplazamiento de ±0,5 h en RA
5. **Imagen 2** — posición final

Cada imagen se resuelve mediante plate solving. Las tres soluciones (RA, DEC en J2000) se convierten en coordenadas horizontales (azimut/altitud) en el instante exacto de la toma, teniendo en cuenta la posición geográfica del observador.

### Cálculo del eje polar

Los tres puntos Az/Alt describen un arco sobre la esfera celeste correspondiente a la rotación de la montura. El módulo calcula el vector normal a este arco — es la dirección del eje de rotación real de la montura.

Los errores se deducen entonces:

| Error | Definición |
|---|---|
| **Error de azimut** | Diferencia entre el azimut del eje y el Norte (azimut = 0°) |
| **Error de altitud** | Diferencia entre la altitud del eje y la latitud del observador |
| **Error total** | Error combinado: `√(az² + alt²)` |

Los errores se muestran en grados y en minutos de arco.

### Fase 2 — Bucle de corrección

Tras la calibración, el módulo entra en un bucle continuo sin desplazamiento de la montura:

1. Adquisición de una imagen
2. Plate solving
3. Cálculo del desplazamiento Az/Alt respecto a la imagen de referencia (imagen 2)
4. Actualización del error residual = error inicial − corrección ya aplicada

El usuario ajusta mecánicamente el azimut y la altitud de la montura; el error residual disminuye en tiempo real a medida que mejora la puesta en estación.

### Superposición gráfica

La imagen mostrada incluye una superposición que representa las correcciones a aplicar:

| Trazo | Color | Significado |
|---|---|---|
| Eje polar → corrección altitud | Amarillo | Error de altitud |
| Corrección altitud → polo | Verde | Error de azimut |
| Eje polar → polo | Rojo | Error total |

El marco está orientado de forma que el Norte celeste esté en la parte superior de la imagen.

## Estados

| Estado | Descripción |
|---|---|
| **Idle** | Módulo inactivo |
| **Moving** | Montura en desplazamiento |
| **Shooting** | Adquisición de imagen en curso |
| **Solving** | Plate solving en curso |
| **Compute** | Cálculo del error polar |

## Valores mostrados

### Mediciones de calibración

| Valor | Descripción |
|---|---|
| **RA 0 / DE 0 / Time 0** | Solución astrométrica y marca de tiempo de la imagen 0 |
| **RA 1 / DE 1 / Time 1** | Solución astrométrica y marca de tiempo de la imagen 1 |
| **RA 2 / DE 2 / Time 2** | Solución astrométrica y marca de tiempo de la imagen 2 |

### Error polar

| Valor | Descripción |
|---|---|
| **Error de azimut (°)** | Error de azimut en grados |
| **Error de altitud (°)** | Error de altitud en grados |
| **Error total (°)** | Error combinado en grados |
| **Error de azimut (arcmin)** | Error de azimut en minutos de arco |
| **Error de altitud (arcmin)** | Error de altitud en minutos de arco |
| **Error total (arcmin)** | Error combinado en minutos de arco |

## Acciones

| Acción | Descripción |
|---|---|
| **Iniciar** | Lanza la secuencia de calibración (3 imágenes + cálculo) y luego el bucle de corrección |
| **Abandonar** | Interrumpe inmediatamente la secuencia en curso |
