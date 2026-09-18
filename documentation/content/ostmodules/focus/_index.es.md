---
title: Focuser
weight: 10
description: Enfoque automático mediante ajuste polinómico y cartografía del tilt del sensor
---

## Función

El módulo Focuser realiza el enfoque automático del telescopio. Desplaza el enfocador motorizado en un rango de posiciones, mide el tamaño de las estrellas (HFR — Half Flux Radius) en cada punto, y luego calcula la posición óptima mediante un ajuste polinómico de grado 2. Con el zoning activado, también produce una cartografía del tilt del sensor.

## Interfaz

La interfaz muestra en tiempo real la curva HFR en construcción, el progreso de la secuencia, y los resultados (HFR final y posición). Los parámetros son accesibles mediante el botón **Parámetros** en la parte superior derecha.

![Captura de pantalla del módulo Focuser](/images/modules/focus.png)

La curva HFR se traza con barras de error que representan la desviación estándar de las mediciones en cada posición, y aparece un ajuste polinómico en línea discontinua naranja a medida que se producen las adquisiciones.

![Curva HFR con barras de error y ajuste polinómico](/images/modules/focus/focus-chart.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| Cámara | Adquisición de las imágenes para la medición HFR |
| Enfocador motorizado | Desplazamiento del enfoque |
| Rueda de filtros | Opcional — usada si está configurada |

## Parámetros

Estos parámetros son accesibles desde el menú **Parámetros** del módulo. Un conjunto de parámetros se puede guardar en un perfil.

### Exposición

| Parámetro | Descripción |
|---|---|
| **Exposición** | Duración de exposición en segundos |
| **Ganancia** | Ganancia de la cámara |
| **Offset** | Offset de la cámara |

### Focus

| Parámetro | Descripción |
|---|---|
| **Posición de inicio** | Posición absoluta del enfocador para iniciar la secuencia (ignorado si *Alrededor de la posición actual* está activado) |
| **Alrededor de la posición actual** | Centra automáticamente la secuencia alrededor de la posición actual del enfocador: `pos_actual − (paso × iteraciones / 2)` |
| **Paso** | Diferencia en unidades de enfocador entre dos mediciones consecutivas |
| **Iteraciones** | Número de puntos de medición. Con 5 iteraciones y un paso de 2000, el enfocador explora un rango de 8000 unidades |
| **Backlash** | Sobrepaso aplicado antes de cada desplazamiento para corregir el juego mecánico. El enfocador retrocede este valor antes de avanzar hacia el objetivo |
| **Zoning** | División de la imagen en zonas (1×1 a 9×9). Ver sección dedicada abajo |

## Algoritmo

El enfoque se desarrolla en dos fases.

### Fase 1 — Exploración

1. El enfocador se posiciona en `posición_inicio − backlash` (para tener en cuenta el juego mecánico)
2. Avanza hasta la `posición_inicio`
3. Para cada iteración:
   - Reinicio del subframe de la cámara
   - Adquisición de una imagen, cálculo del HFR y de su desviación estándar
   - Registro del triplete (posición, HFR, desviación estándar)
   - Actualización del ajuste polinómico en tiempo real
   - Avance de un *paso*

El ajuste polinómico de grado 2 da de forma continua la posición del mínimo de la curva HFR:

```
posición_óptima = −k1 / (2 × k2)    [para k0 + k1·x + k2·x²]
```

### Fase 2 — Posicionamiento final

1. El enfocador retrocede hacia `mejor_posición − backlash`
2. Avanza hasta la posición óptima calculada por el polinomio
3. Se adquiere una imagen final para medir el HFR resultante

{{% notice style="tip" title="Valor de retorno" %}}
La posición óptima retenida es la del **ajuste polinómico** (`bestposfit`). Si el ajuste no pudo calcularse (menos de 3 puntos), se utiliza la mejor posición medida directamente (`bestpos`).
{{% /notice %}}

## Curva HFR

La curva HFR se traza en tiempo real durante la exploración y se actualiza en cada punto de medición. Consta de dos elementos superpuestos:

- **Puntos medidos** (verde) con **barras de error** que representan ±1 desviación estándar de las mediciones en cada posición
- **Curva polinómica** (línea discontinua naranja) — ajuste de grado 2 actualizado en cada nuevo punto

El eje X representa la posición absoluta del enfocador, el eje Y el HFR en píxeles.

{{% notice style="note" title="Barras de error" %}}
La desviación estándar mostrada corresponde a la dispersión de los HFR individuales medidos por estrella en una misma posición de enfoque. Una barra de error amplia indica un seeing variable o una turbulencia importante en el momento de la adquisición.
{{% /notice %}}

## Zoning

El zoning divide la imagen en una cuadrícula de N×N zonas y calcula un ajuste polinómico independiente para cada zona. Permite tanto un análisis fino de las variaciones HFR en el campo como una cartografía del tilt del sensor.

| Valor | Cuadrícula | Uso |
|---|---|---|
| 1 | 1×1 | Sin zoning — una sola medición global |
| 2 | 2×2 | Detección básica del tilt (4 cuadrantes) |
| 3 | 3×3 | Análisis habitual — centro + bordes + esquinas |
| 5 | 5×5 | Análisis fino |
| 9 | 9×9 | Análisis muy fino — 81 zonas |

Para cada zona, el módulo calcula mediante el algoritmo de Welford (actualización incremental en una sola pasada):
- El **HFR medio** de la zona
- La **desviación estándar HFR** de la zona — indicador de la calidad óptica local

## Cartografía del tilt del sensor

Cuando el zoning está activado (≥ 2×2), el módulo genera automáticamente cuatro imágenes de cartografía al final de la secuencia. Cada una representa la posición de enfoque óptima de cada zona, codificada en color (azul = posición baja / plano cercano, rojo = posición alta / plano lejano).

![Ejemplos de cartografías del tilt del sensor (4 imágenes)](/images/modules/focus/focus-tilt-maps.png)

La relación ancho/alto de las imágenes corresponde a la del sensor.

### rawTilt — Tilt bruto por zona

Cada zona se representa mediante un bloque de color uniforme proporcional a la posición de mejor enfoque. Sin interpolación — la resolución espacial es la de la cuadrícula de zoning.

![rawTilt — bloques de color por zona](/images/modules/focus/focus-rawtilt.png)

### localTiltLinear — Interpolación bilineal local

Interpolación bilineal entre los centros de las zonas adyacentes. Produce un mapa suave sin suposición sobre la forma global del plano focal.

### globalTiltLinear — Plano global (ajuste lineal GSL)

Ajuste de un plano por mínimos cuadrados (biblioteca GSL) sobre el conjunto de las posiciones de zona. Ideal para cuantificar un tilt puro: inclinación del sensor respecto al eje óptico. El residuo mostrado en cada celda (en amarillo) es la diferencia entre la medición y el plano ajustado.

### globalTiltQuadratic — Superficie cuadrática (ajuste GSL grado 2)

Ajuste de una superficie de segundo grado. Captura los defectos no planos: curvatura de campo, flexión del sensor. La gama de colores cubre el mínimo y el máximo reales de la superficie (vértice incluido si se encuentra dentro de los límites del sensor).

{{% notice style="note" title="Interpretación" %}}
- **rawTilt / localTiltLinear**: diagnósticos locales, útiles para detectar una zona anómala debida a una mala estrella.
- **globalTiltLinear**: medición directa del tilt. Si el gradiente es uniforme, inclinar el sensor en la dirección indicada bastará para corregirlo.
- **globalTiltQuadratic**: si el residuo del plano global es importante y la superficie cuadrática es claramente mejor, la curvatura de campo o una flexión mecánica son la causa.
{{% /notice %}}

## Valores mostrados en tiempo real

| Valor | Descripción |
|---|---|
| **Estado** | Estado actual de la secuencia (Idle / Running / Ok / Error) |
| **Progreso** | Barra de progreso global de la secuencia |
| **HFR** | HFR medido en la posición final |
| **Posición** | Posición final del enfocador |

## Resultado final

Al finalizar el enfoque, el módulo publica:

| Resultado | Descripción |
|---|---|
| **Posición final** | Posición absoluta del enfocador tras el posicionamiento |
| **HFR final** | HFR medido en la posición óptima |

Estos valores también se publican hacia otros módulos (secuenciador) mediante el sistema de eventos entre módulos.

## Acciones

| Acción | Descripción |
|---|---|
| **Lanzar el enfoque** | Inicia la secuencia completa |
| **Abandonar** | Interrumpe inmediatamente la secuencia en curso |
