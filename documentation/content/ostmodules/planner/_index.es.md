---
title: Planner
weight: 50
description: Planificación multiobjetivo y orquestación automática de la sesión
---

## Función

El módulo Planner orquesta una sesión de astrofotografía multiobjetivo. Ejecuta una lista de líneas de planificación, delegando sucesivamente el apuntado al módulo Navigator y la adquisición al módulo Secuenciador. Para cada objetivo, carga automáticamente un perfil de secuencia. Antes de apuntar a un objetivo, también verifica que permanezca por encima de una elevación mínima durante toda la duración estimada de la secuencia y que esté suficientemente alejado de la luna, saltando y reintentando más tarde los objetivos que no respeten estas restricciones.

![Captura de pantalla del módulo Planner](/images/modules/planner.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| GPS | Suministro de las coordenadas geográficas y de la hora |

## Estructura de una planificación

Una planificación se compone de varias líneas. Cada línea define:

| Columna | Descripción |
|---|---|
| **Objeto** | Nombre del objetivo |
| **RA** | Ascensión recta del objetivo (J2000) |
| **DEC** | Declinación del objetivo (J2000) |
| **Perfil** | Perfil de secuencia a cargar en el Secuenciador para este objetivo |
| **Progreso** | Avance de la línea en curso |

Las líneas no se ejecutan necesariamente en estricto orden de arriba a abajo: una línea que falla en el control de visibilidad/luna se salta y se reintenta más tarde, una vez que todas las demás líneas hayan pasado (ver *Estados de una línea* abajo). Se pueden añadir manualmente o enviar desde el Navigator mediante la acción *Añadir a la planificación*.

## Parámetros

### Módulos esclavos

| Parámetro | Descripción |
|---|---|
| **Instancia del Secuenciador** | Nombre de la instancia del módulo Secuenciador a pilotar |
| **Instancia del Navigator** | Nombre de la instancia del módulo Navigator a pilotar |

### Restricciones de visibilidad

| Parámetro | Descripción |
|---|---|
| **Minimum elevation (°)** | El objetivo debe permanecer por encima de esta elevación durante toda la duración estimada de la secuencia, de lo contrario la línea se salta |
| **Moon illumination threshold (%)** | La evitación de la luna solo se aplica cuando su fracción iluminada supera este porcentaje |
| **Moon separation threshold (°)** | La línea solo se rechaza si el objetivo está más cerca que esta distancia **y** la iluminación de la luna supera el umbral anterior |
| **Max retries before giving up** | Una línea que falla en el control de visibilidad/luna este número de veces pasa al estado *Failed* y ya no se reintenta nunca |

## Algoritmo

### Inicio

1. Verificación de la conexión INDI y de los módulos esclavos
2. Marcado de todas las líneas en estado *En espera* y reinicio de los contadores de intentos
3. Inicio de la primera línea

### Ejecución de una línea

Para cada línea de la planificación:

1. El Planner solicita al Secuenciador la duración teórica del perfil solicitado (sin cargarlo), mediante una petición genérica entre módulos
2. Verifica que el objetivo permanece por encima de la elevación mínima configurada durante toda esta duración, y suficientemente lejos de la luna (umbrales de iluminación/distancia) — si uno de los dos controles falla, la línea se salta (ver *Estados de una línea*) y el Planner pasa a la siguiente
3. En caso contrario, configura el objetivo del Navigator (RA, DEC, nombre)
4. Carga el perfil de secuencia solicitado en el Secuenciador y configura el objeto correspondiente
5. Desencadena la acción **Goto objetivo** del Navigator
6. Espera el fin del centrado (evento de éxito del Navigator)
7. Desencadena la acción **Lanzar la secuencia** del Secuenciador
8. Sigue el progreso de la secuencia y espera su fin
9. La línea se marca *Terminada* y el Planner pasa a la siguiente línea utilizable

### Estados de una línea

| Estado | Significado | ¿Se reintenta más tarde? |
|---|---|---|
| **Queued** | Aún no intentada | — |
| **Checking visibility** | Control de elevación/luna en curso | — |
| **Slewing** | Centrado del Navigator sobre el objetivo | — |
| **Skipped: \<razón\>** | Fallo del control de visibilidad/luna | Sí, una vez que todas las demás líneas hayan pasado |
| **Failed: \<razón\>** | Fallo del control de visibilidad/luna demasiadas veces (*Max retries* alcanzado) | No — excluida definitivamente |
| **Cancelled** | La secuencia se interrumpió manualmente durante su ejecución (a diferencia de un fin normal) | No — excluida definitivamente |
| **Finished** | Secuencia terminada con éxito | No |

{{% notice style="tip" title="Cancelación manual frente a fin normal" %}}
El Planner distingue los dos casos: si interrumpes directamente el Secuenciador mientras ejecuta una línea de la planificación, esa línea se marca *Cancelled* y el Planner pasa a la siguiente línea utilizable — no se confunde con un éxito, y no se reintenta después.
{{% /notice %}}

### Fin de la planificación

El Planner sigue recorriendo en bucle las líneas, reintentando las que aún están *Skipped*, hasta que cada línea esté ya sea *Finished*, *Failed*, o *Cancelled*. En ese momento, el progreso global pasa al 100 % y el módulo se detiene.

### Propagación de la detención

Detener el Planner (ya sea mediante el botón **Detener**, o porque un módulo padre como el Park Manager lo solicita) también interrumpe sus módulos esclavos: envía la acción **Interrumpir** del Secuenciador y la acción **Interrumpir** del Navigator, para que una planificación interrumpida no deje una exposición o un apuntado continuar sin supervisión.

## Progreso

| Indicador | Descripción |
|---|---|
| **Progreso global** | Número de líneas procesadas sobre el total |
| **Progreso por línea** | Avance de la adquisición en curso, retransmitido desde el Secuenciador |

## Acciones

| Acción | Descripción |
|---|---|
| **Iniciar** | Lanza la ejecución de la planificación desde la primera línea |
| **Detener** | Interrumpe la planificación en curso |
