---
title: Secuenciador
weight: 30
description: Adquisición automática de secuencias de imágenes
---

## Función

El módulo Secuenciador orquesta la adquisición automática durante una sesión de astrofotografía. Ejecuta una lista de líneas de secuencia, cada una definiendo el filtro, la duración de exposición y el número de imágenes a adquirir. El tipo de trama, la ganancia y el offset ahora son compartidos por toda la secuencia en lugar de definirse línea por línea. Los archivos FITS se guardan automáticamente en una estructura de carpetas organizada por objeto y por filtro, y el módulo puede solicitar la refrigeración de la cámara, el enfoque automático y el guiado alrededor de cada línea según las necesidades.

![Captura de pantalla del módulo Secuenciador](/images/modules/sequencer.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| Cámara | Adquisición de las imágenes, y refrigeración si está activada |
| Rueda de filtros | Opcional — la secuencia funciona sin ella; el posicionamiento y los perfiles de enfoque por filtro simplemente se ignoran |

## Estructura de una secuencia

Una secuencia se compone de varias líneas. Cada línea define:

| Columna | Descripción |
|---|---|
| **Filtro** | Filtro seleccionado en la rueda de filtros |
| **Exposición** | Duración de exposición en segundos |
| **Número** | Número de imágenes a adquirir para esta línea |

Las líneas se ejecutan en orden, de la primera a la última. El tipo de trama, la ganancia y el offset se aplican a todas las líneas (ver *Parámetros de secuencia* abajo).

## Parámetros

Los ajustes se reparten en tres grupos, para no mezclar el comportamiento de secuencia, de guiado y de enfoque.

### Objeto

| Parámetro | Descripción |
|---|---|
| **Nombre** | Nombre del objeto fotografiado — usado para nombrar la carpeta de guardado |
| **AR** | Ascensión recta del objeto |
| **DEC** | Declinación del objeto |

### Módulos esclavos

El secuenciador interactúa con otros módulos para el enfoque y el guiado.

| Parámetro | Descripción |
|---|---|
| **Instancia del módulo de enfoque** | Nombre de la instancia del módulo Focuser a usar (ej. `focus`) |
| **Instancia del módulo de guiado** | Nombre de la instancia del módulo Guider a usar (ej. `guider`) |

### Parámetros de secuencia

| Parámetro | Descripción |
|---|---|
| **Activar la refrigeración de la cámara** | Solicita la refrigeración a la temperatura objetivo antes de iniciar la secuencia, y espera a que se alcance |
| **Temperatura objetivo (°C)** | Temperatura objetivo del sensor |
| **Tipo de trama** | Light (L), Bias (B), Dark (D), o Flat (F) — se aplica a todas las líneas |
| **Ganancia** | Ganancia de la cámara — se aplica a todas las líneas |
| **Offset** | Offset de la cámara — se aplica a todas las líneas |

### Parámetros de guiado

| Parámetro | Descripción |
|---|---|
| **Usar el guiado** | Activa la integración con el guiador (dithering, estabilización, supervisión del RMS, suspensión durante el enfoque). El guiador debe estar ya guiando, o ser iniciado por el secuenciador si es necesario |
| **Tiempo de estabilización tras reanudar el guiado (s)** | Tiempo de espera en segundos tras reanudar el guiado, antes de continuar las adquisiciones |
| **Dithering cada N tomas (0=desactivado)** | Solicita un dithering cada N imágenes capturadas |
| **Umbral de recalibración RMS (0=desactivado)** | Interrumpe la exposición en curso y desencadena una recalibración del guiador cuando el RMS máximo durante esta exposición supera este valor |

### Parámetros de enfoque

| Parámetro | Descripción |
|---|---|
| **Enfoque automático al cambiar de filtro** | Desencadena un enfoque en cada cambio de filtro — esto también cubre la primera línea de la secuencia, ya que aún no se ha realizado ningún enfoque en esta sesión del módulo. Relanzar la misma secuencia evita un enfoque redundante si el último realizado ya corresponde al filtro actual |
| **Suspender el guiado durante el enfoque** | Pausa el guiado antes del enfoque y lo reanuda después |
| **Umbral de re-enfoque HFR (0=desactivado)** | Desencadena un enfoque adicional durante la línea cuando el HFR medido supera este umbral |
| **Perfil de enfoque a cargar** | Perfil a cargar en el módulo de enfoque antes de solicitar el autofocus; dejar vacío para conservar el perfil actualmente cargado |

### Perfil de enfoque por filtro

Una cuadrícula dedicada permite sobrescribir el perfil de enfoque para filtros específicos — útil, por ejemplo, cuando los filtros de banda estrecha necesitan ajustes de enfoque distintos de los filtros LRGB.

| Columna | Descripción |
|---|---|
| **Filtro** | Filtro al que se aplica esta línea |
| **Perfil de enfoque** | Perfil a cargar al enfocar en este filtro |

Si el filtro actual tiene una línea en esta cuadrícula, se usa su perfil. En caso contrario, el secuenciador recurre a *Perfil de enfoque a cargar* arriba. Si este también está vacío, el módulo de enfoque conserva el perfil que tiene actualmente cargado.

## Algoritmo

### Inicio de la secuencia

1. Conexión a la cámara y reinicio de la trama
2. Si *Activar la refrigeración de la cámara* está activo, solicita la temperatura objetivo y espera a que se alcance (se ignora inmediatamente si está desactivado o si la cámara no está configurada)
3. Inicio de la primera línea

### Ejecución de una línea

Para cada línea de la secuencia:

1. Selección del filtro en la rueda de filtros (se ignora si no hay ninguna rueda de filtros configurada)
2. Si *Enfoque automático al cambiar de filtro* está activado y el filtro difiere del usado en el último enfoque de esta sesión: se desencadena el enfoque automático (cargando primero un perfil — sobrescritura por filtro, luego repliegue general — y suspendiendo/reanudando el guiado si está configurado)
3. Inicio o confirmación del guiado si *Usar el guiado* está activado
4. Dithering si se alcanza el intervalo de tomas configurado
5. Adquisición del número de imágenes definido una por una, con re-enfoque durante la línea si se supera el umbral HFR

### Guardado de los archivos

Los archivos FITS se guardan en la siguiente estructura (los segmentos de objeto y filtro se omiten cuando no están definidos, en lugar de dejar separadores superfluos):

```
<objeto>/
  LIGHT/
    <filtro>/   ← Tramas Light
  FLAT/
    <filtro>/   ← Tramas Flat
  BIAS/         ← Tramas Bias
  DARK/         ← Tramas Dark
```

Cada nombre de archivo incluye el nombre del objeto, el tipo de trama, el filtro y una marca de tiempo.

### Integración con el módulo de enfoque

Cuando se solicita un enfoque automático:

1. Si se aplica un perfil (sobrescritura por filtro, o repliegue general), el secuenciador solicita al módulo Focuser que lo cargue
2. El secuenciador envía la acción `autofocus` al módulo Focuser designado
3. Pausa las adquisiciones y espera el fin del enfoque
4. Si *Suspender el guiado* está activado, envía `abortguider` al módulo Guider antes del enfoque, y luego `guide` después
5. Si se configura un tiempo de estabilización, espera este plazo antes de reanudar

### Integración con el módulo de guiado

- El guiado se inicia (o se confirma su presencia) antes del inicio de las exposiciones, cuando *Usar el guiado* está activado
- Se solicita un dithering cada N tomas, y luego el secuenciador espera el tiempo de estabilización configurado
- Si el RMS máximo durante una exposición supera el umbral configurado, la exposición se interrumpe y el guiador se recalibra antes de continuar

## Progreso

El progreso se sigue tanto en directo (actualizado durante la adquisición) como en forma de promedios/duraciones acumuladas desde la carga del módulo.

| Indicador | Descripción |
|---|---|
| **Secuencia (tomas)** | Número de tomas terminadas sobre el total de la secuencia completa |
| **Secuencia (tiempo)** | Tiempo de exposición transcurrido sobre el tiempo de exposición total estimado, actualizado en directo durante la exposición en curso |
| **Exposición en curso** | Progreso de la exposición en curso (0 a 100 %) |
| **Duración total teórica** | Suma de número × exposición sobre todas las líneas, recalculada en cada modificación de la cuadrícula — incluso antes de iniciar la secuencia |
| **Duración restante teórica** | Duración total teórica menos el tiempo ya transcurrido |
| **Duración real transcurrida** | Cronómetro real, reiniciado en cada inicio de la secuencia |
| **Duración media de enfoque** | Media móvil del tiempo pasado esperando el autofocus, desde la carga del módulo |
| **Duración media de inicio del guiado** | Media móvil del tiempo pasado esperando el inicio del guiador, desde la carga del módulo |

## Acciones

| Acción | Descripción |
|---|---|
| **Iniciar la secuencia** | Inicia la ejecución de la secuencia desde la primera línea |
| **Interrumpir** | Detiene inmediatamente la secuencia en curso, y también propaga una solicitud de interrupción a las instancias Guider y Focuser configuradas en *Módulos esclavos*, para detener también el guiado y un posible enfoque en curso |
