---
title: Guider
weight: 20
description: Guiado automático por corrección de deriva
---

## Función

El módulo Guider realiza el guiado automático del telescopio. Detecta y sigue un campo de estrellas de referencia, mide la deriva en tiempo real y envía impulsos de corrección a la montura para mantener el apuntado.

![Captura de pantalla del módulo Guider](/images/modules/guider.png)

## Equipos requeridos

| Equipo | Función |
|---|---|
| Cámara de guiado | Adquisición de las imágenes del campo de guiado |
| Interfaz de guiado | Recibe los impulsos temporizados (`TELESCOPE_TIMED_GUIDE_*`) — la propia montura, o una caja que retransmite impulsos ST4 |

## Fases de funcionamiento

El guiado se desarrolla en tres fases sucesivas.

### Fase 1 — Inicialización

El módulo adquiere una imagen de referencia y conserva la lista de estrellas detectadas como referencia para toda la sesión de guiado. Cuando el guiado se inicia con una calibración guardada válida, solo esta fase precede al guiado: se toma una nueva referencia en el apuntado actual, lo que permite reutilizar la calibración tras un desplazamiento de la montura.

### Fase 2 — Calibración

El módulo mide la respuesta de la montura a los impulsos de corrección.

1. Se envía una serie fija de impulsos al Oeste, luego al Este, luego al Norte, luego al Sur (`Paso de calibración` impulsos por dirección).
2. Se mide el desplazamiento imagen por impulso de cada pasada respecto a la imagen anterior.
3. **La tasa de impulso se obtiene de las combinaciones antisimétricas `(Oeste − Este) / 2` y `(Norte − Sur) / 2`.** Una pasada unidireccional está contaminada por la deriva de fondo constante (error de seguimiento sideral, error periódico, deriva de la puesta en estación); la diferencia de las dos pasadas opuestas anula esta deriva y proporciona una tasa RA limpia y una tasa DEC limpia. Las tasas brutas por dirección se siguen mostrando, a título de diagnóstico.
4. La orientación del sensor se deduce de los dos ejes (una estimación por el eje RA y otra por el eje DEC, promediadas), resolviéndose la ambigüedad de 180° respecto al eje RA.
5. La tasa RA se reduce al ecuador (`tasa × cos(DEC de calibración)`) para poder transponerla a cualquier declinación en el momento del guiado.

Antes de la serie Norte y antes de la serie Sur, un impulso opcional de *recuperación de juego DEC* toma primero el juego mecánico del eje DEC, para que los impulsos de medición temporizados no se pierdan en parte al reengranar. Su propio desplazamiento se excluye del cálculo de la tasa.

El resultado de la calibración se guarda y se reutiliza de una sesión a otra, siempre que el lado del pie y la orientación de la cámara no hayan cambiado.

{{% notice style="note" title="Calidad de la calibración" %}}
Tras las cuatro pasadas, el módulo calcula un *ratio de deriva* — la magnitud de la deriva de fondo constante en relación con el desplazamiento debido a los impulsos — y un indicador **Calidad de calibración** (verde / naranja / rojo). Si el ratio supera el *Ratio de deriva de fondo máx*, la calibración se rechaza y se relanza, hasta *Número de intentos de calibración* veces, y luego se abandona con un mensaje que invita a comprobar la puesta en estación. Una calibración también se abandona si la estrella se aleja más de *Excursión de estrella máx* de su posición inicial, incluso antes de poder salir del sensor.
{{% /notice %}}

### Fase 3 — Guiado

Bucle de corrección continua:

1. Adquirir una imagen.
2. Seguir el campo de estrellas respecto a la referencia (ver *Seguimiento de estrellas* abajo).
3. Calcular la deriva, proyectada sobre los ejes RA y DEC con la orientación de calibración.
4. **Regulación PI** — la corrección vale `agresividad × deriva + ganancia integral × deriva acumulada`. El término proporcional solo no puede anular una deriva constante (se estabiliza en `deriva / agresividad`); el término integral sigue creciendo hasta que el residuo sea realmente nulo. El acumulador está acotado (*Límite de la integral*) y se congela para un eje cuyo impulso ya está al máximo.
5. La tasa RA se adapta a la declinación actual: `tasa ecuatorial / cos(DEC actual)`.
6. Si la *Compensación de juego DEC* está activa y la dirección DEC se ha invertido desde la última corrección, la estimación de compensación actual se añade por encima.
7. Los impulsos de corrección se envían a la montura, limitados a *Impulso mín* / *Impulso máx*.
8. Se actualizan las estadísticas (RMS RA, DEC, total).
9. Si se envió un impulso de compensación de inversión en el ciclo anterior, la reducción de deriva realmente obtenida se compara con la predicción y la estimación de compensación se ajusta al alza (subcorrección) o a la baja (sobrecorrección), dentro de los límites configurados.

La **primera imagen** de una sesión se mide pero no se corrige — solo fija el cero de trabajo, para que unos pocos píxeles de estabilización tras el desplazamiento no desencadenen un primer impulso grande.

Una pérdida pasajera de correlación de estrellas simplemente **salta** la imagen (ningún impulso); el guiado solo se abandona tras *Imágenes perdidas consecutivas máx* seguidas. Un **watchdog de hardware** abandona la sesión si una exposición, un impulso, un reinicio de trama o una extracción de estrellas nunca termina dentro del plazo *Plazo de hardware*.

{{% notice style="note" title="Compensación de juego DEC" %}}
Los engranajes de la montura tienen un juego mecánico: en cada inversión de dirección del guiado DEC, los primeros impulsos toman en parte este juego en lugar de producir un movimiento real, de ahí un pico de deriva temporal. El juego real varía con la posición de apuntado y la carga, por lo que un valor fijo subcorrige o sobrecorrige. La estimación adaptativa se autoajusta: cada inversión es a la vez una corrección y una medición, por lo que converge hacia el juego realmente presente.
{{% /notice %}}

## Seguimiento de estrellas

Entre dos imágenes, el campo de estrellas sufre una traslación casi pura (la rotación de 180° de una inversión al meridiano se trata por separado mediante los indicadores de inversión). El seguimiento busca directamente la **traslación dominante**:

1. Cada par (estrella de referencia, estrella actual) implica una traslación candidata; la verdadera es aquella en la que más estrellas coinciden (etapa de votación / RANSAC).
2. Sus puntos coherentes se refinan mediante una ventana de dos etapas y un recorte sigma iterativo, y la deriva es una media de los residuos ponderada por el flujo — continua y subpíxel.

Este método es insensible a un cambio en la clasificación de las estrellas por flujo y a la aparición o desaparición de estrellas de una imagen a otra. Los ajustes asociados están en *Emparejamiento de estrellas* dentro de los parámetros.

{{% notice style="note" title="Transposición en declinación de la tasa RA" %}}
Un impulso desplaza la estrella `tasa × impulso × cos(DEC)` sobre el sensor, por lo que los ms/px varían en `1 / cos(DEC)`. La tasa RA se registra por tanto reducida al ecuador en el momento de la calibración y se divide por `cos(DEC actual)` en el guiado, para que una calibración hecha en una declinación siga siendo correcta en otra.
{{% /notice %}}

## Parámetros

Los parámetros son accesibles desde el menú **Parámetros** del módulo y un conjunto se puede guardar como perfil.

### Equipos

| Parámetro | Descripción |
|---|---|
| Exposición | Duración de exposición en segundos |
| Ganancia | Ganancia de la cámara |
| Offset | Offset de la cámara |

### Calibración

| Parámetro | Descripción |
|---|---|
| **Duración de impulso** | Duración en ms de cada impulso de calibración |
| **Paso de calibración** | Número de impulsos por dirección |
| **Recuperación de juego DEC** | Impulso adicional (ms) antes de las series Norte y Sur, para tomar primero el juego mecánico — 0 para desactivar |
| **Ratio de deriva de fondo máx** | Rechazar la calibración si la deriva de fondo constante supera esta fracción del desplazamiento debido a los impulsos — un valor elevado suele indicar una puesta en estación mediocre. 0 para desactivar el control |
| **Número de intentos de calibración** | Cuántas veces relanzar una calibración rechazada antes de abandonar |
| **Excursión de estrella máx** | Abandonar la calibración si la estrella se aleja más de esta fracción de la dimensión más pequeña de la imagen respecto a su inicio. 0 para desactivar |

### Guiado

| Parámetro | Descripción |
|---|---|
| **Impulso máx** | Duración máxima de un impulso de corrección (ms) |
| **Impulso mín** | Duración mínima de un impulso de corrección (ms) |
| **Agresividad RA** | Fracción proporcional de la corrección RA calculada que se aplica |
| **Agresividad DEC** | Fracción proporcional de la corrección DEC calculada que se aplica |
| **Ganancia integral RA** | Ganancia sobre el residuo RA acumulado — anula la desviación permanente frente a una deriva constante. 0 = puramente proporcional |
| **Ganancia integral DEC** | Idem, para el eje DEC |
| **Límite de la integral (px)** | Anti-saturación: amplitud a la que se limita el acumulador integral. 0 para desactivar el límite |
| **Umbral RMS** | Multiplicador del RMS actual por encima del cual una corrección se descarta |
| **RMS calculado sobre x** | Número de imágenes sobre el cual se calcula el RMS |
| **Inversión según lado del pie** | Invertir automáticamente las correcciones RA/DEC cuando el lado del pie difiere del de la calibración |
| **Amplitud de dither (px)** | Desplazamiento aleatorio máximo aplicado durante un dither |
| **Imágenes perdidas consecutivas máx** | Saltar una imagen de guiado en caso de pérdida de correlación; abandonar solo tras este número seguidas |
| **Plazo de hardware (s)** | Abandonar si la cámara o la montura dejan de responder durante este tiempo. 0 para desactivar el watchdog |

### Emparejamiento de estrellas

| Parámetro | Descripción |
|---|---|
| **Ventana de emparejamiento (px)** | Radio de búsqueda al emparejar una estrella de referencia en la imagen actual |
| **Estrellas emparejadas mín** | Número mínimo de estrellas de referencia a encontrar para que la deriva se considere fiable |
| **Estrellas usadas máx** | Límite del número de estrellas más brillantes retenidas en cada imagen |

### Compensación de juego DEC

| Parámetro | Descripción |
|---|---|
| **Activar** | Añadir un impulso adicional adaptativo en cada inversión de dirección del guiado DEC |
| **Compensación actual (ms)** | Estimación adaptativa, ajustada tras cada inversión — fijar un valor inicial o dejarlo en 0 para que se aprenda |
| **Compensación mínima / máxima (ms)** | Límites de la estimación adaptativa |
| **Paso de ajuste (ms)** | Cuánto se retoca la estimación tras cada inversión, según la sub/sobrecorrección medida |

### Inversiones manuales

| Parámetro | Descripción |
|---|---|
| **Invertir RA** | Invertir manualmente la dirección de corrección RA |
| **Invertir DEC** | Invertir manualmente la dirección de corrección DEC |

### Desactivación de correcciones

| Parámetro | Descripción |
|---|---|
| **Desactivar RA+ / RA− / DEC+ / DEC−** | Desactivar los impulsos de corrección en esta dirección |

### Ayudas de prueba

| Parámetro | Descripción |
|---|---|
| **SIM: falsa inversión al meridiano** | Añade 180° al ángulo de calibración al iniciar el guiado (el efecto que tiene una inversión real sobre la correspondencia deriva → RA/DEC), para probar la gestión de las inversiones de corrección con el simulador CCD, que nunca hace girar su propio campo. Dejar desactivado en uso real |

## Valores de calibración

| Valor | Descripción |
|---|---|
| **Calidad de calibración** | Indicador: verde (limpia) / naranja (utilizable, deriva de fondo notable) / rojo (rechazada) |
| **Tasa RA (ms/px)** | Tasa RA ecuatorial utilizada para el guiado (Oeste/Este combinados, deriva eliminada) |
| **Tasa DEC (ms/px)** | Tasa DEC utilizada para el guiado (Norte/Sur combinados) |
| **bruto N / S / E / W (ms/px)** | Tasas brutas por dirección, solo diagnóstico (aún llevan la deriva de fondo) |
| **Orientación CCD** | Ángulo de calibración (grados) |
| **DEC montura en la calibración** | Declinación de la montura en el momento de la calibración |
| **Invertir RA / DEC** | Inversiones de corrección registradas durante la calibración |

## Valores en directo

| Valor | Descripción |
|---|---|
| **Impulso N / S / E / W** | Duración del último impulso en cada dirección (ms) |
| **RMS RA / DEC / total** | Error cuadrático medio de la deriva (píxeles) |

## Gráficos

### Deriva (nube de puntos)

Nube XY de la deriva de cada corrección respecto a la posición de referencia, con círculos de tolerancia.

### Guiado (serie temporal)

Gráfico de estilo PHD2 que muestra, por corrección:

| Curva | Descripción |
|---|---|
| Deriva RA / deriva DEC | Deriva medida en cada eje (píxeles) |
| Impulso RA / impulso DEC | Duración del impulso de corrección (ms) |
| SNR | Relación señal/ruido de la detección de estrellas |
| RMS | RMS total actual (píxeles) |

## Acciones

| Acción | Descripción |
|---|---|
| **Calibrar** | Inicialización y luego solo calibración |
| **Guiar** | Iniciar el guiado — utiliza la calibración guardada, o calibra primero si no hay ninguna |
| **Abandonar** | Detener inmediatamente la secuencia en curso |
| **Reiniciar la calibración** | Borrar los datos de calibración guardados |
| **Dither** | Desplazar aleatoriamente la posición de guiado hasta *Amplitud de dither* |
