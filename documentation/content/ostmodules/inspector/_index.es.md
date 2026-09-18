---
title: Inspector
weight: 70
description: Análisis de la calidad óptica (HFR, forma de las estrellas) y asistente de colimación
---

## Función

El módulo Inspector realiza dos análisis independientes sobre un campo de estrellas. Cada uno se activa por separado — ninguno, uno, el otro, o ambos a la vez:

- **Análisis Inspector** — calidad óptica sobre el conjunto de la imagen, sobre una imagen *enfocada*. Produce tres mapas complementarios (HFR por zona, elongación de las estrellas, mosaico de esquinas) que permiten diagnosticar defectos ópticos como el tilt, el coma o la curvatura de campo.
- **Análisis Collimator** — colimación óptica (alineación de los espejos) de los telescopios Newton, RC (Ritchey-Chrétien) y SC (Schmidt-Cassegrain), sobre una imagen *desenfocada*. Analiza el conjunto del campo de estrellas y muestra continuamente la desviación de colimación y las correcciones por tornillo. En modo semimanual: el software desenfoca, captura y analiza en bucle mientras el usuario actúa físicamente sobre los tornillos de colimación (no motorizados) observando la retroalimentación visual actualizarse casi en tiempo real.

El análisis puede realizarse sobre una imagen adquirida en directo desde la cámara, o sobre un archivo FITS cargado desde el disco.

![Captura de pantalla del módulo Inspector](/images/modules/inspector.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| Cámara | Adquisición de las imágenes (opcional si se carga desde archivo) |
| Focuser (opcional) | Control automático del desenfoque para el flujo de colimación, y función de posición «home». Si no está presente, desenfoque manual antes de cada captura |

## Selección de los análisis

| Interruptor | Efecto |
|---|---|
| **Análisis Inspector** | Lanza el análisis HFR / forma / esquinas en cada imagen capturada (o recargada) |
| **Análisis Collimator** | Lanza el análisis de colimación en cada imagen capturada (o recargada) |

Ambos interruptores son independientes (no exclusivos) y se guardan con el perfil del módulo. Cuando un análisis se desactiva, sus imágenes de resultado se borran para no mostrar nunca un mapa obsoleto junto a un mapa reciente procedente de otra imagen.

## Parámetros

### Adquisición

| Parámetro | Descripción |
|---|---|
| **Exposición** | Duración de exposición en segundos |
| **Ganancia** | Ganancia de la cámara |
| **Offset** | Offset de la cámara |
| **Focal** | Distancia focal óptica en mm — usada para convertir los HFR y el vector de colimación en segundos de arco |
| **Tamaño de las esquinas (píxeles)** | Anchura en píxeles de los parches extraídos para el mosaico de esquinas |

### Análisis

| Parámetro | Descripción |
|---|---|
| **Zoning** | Cuadrícula N×N de división de la imagen para el análisis por zona (de 1×1 a 64×64) |

### Desenfoque

| Parámetro | Descripción |
|---|---|
| **Offset del focuser (pasos)** | Desplazamiento del focuser (en pasos) aplicado por *Go intra* / *Go extra* para alcanzar la posición desenfocada |

## Mapas producidos (análisis Inspector)

### Mapa HFR

El mapa HFR divide la imagen en una cuadrícula N×N (según el parámetro *Zoning*). Para cada zona, el HFR medio se calcula sobre todas las estrellas detectadas, y luego se suaviza mediante una media de las 8 celdas vecinas.

El color de cada zona va del verde (mejor HFR) al rojo (peor HFR).

En superposición:
- Círculos azules alrededor de cada estrella detectada, de radio proporcional al HFR individual
- Rectángulo blanco que conecta los centros de los cuatro cuadrantes, cuya forma refleja la distribución del HFR: un cuadrado perfecto indica una uniformidad ideal
- Valor HFR de cada cuadrante (arriba-izquierda, arriba-derecha, abajo-izquierda, abajo-derecha) y HFR global, expresados en segundos de arco

### Mapa de forma (aberraciones)

El mapa de forma analiza la elongación de las estrellas por zona. Para cada zona de la cuadrícula, se dibuja un trazo:

- **Orientación**: dirección del eje principal de las estrellas (ángulo medio)
- **Longitud**: proporcional a la elongación `a/b − 1` (relación eje mayor / eje menor)

Un trazo corto y uniforme en toda la imagen indica estrellas redondas. Trazos largos en la periferia señalan coma o astigmatismo.

### Mosaico de esquinas

El mosaico ensambla nueve parches de tamaño *Tamaño de las esquinas* extraídos de nueve posiciones de la imagen original, dispuestos en una cuadrícula 3×3:

```
Arriba-izquierda  |  Arriba-centro   |  Arriba-derecha
Medio-izquierda   |  Centro          |  Medio-derecha
Abajo-izquierda   |  Abajo-centro    |  Abajo-derecha
```

Esta vista permite comparar directamente la calidad de las estrellas en el centro y en los bordes del campo.

## Análisis de colimación

Una estrella desenfocada aparece como un anillo ("donut") cuya forma y centrado dan información sobre la colimación. A diferencia de un enfoque clásico que compara una única estrella en intra-focal y en extra-focal, el análisis de colimación aprovecha **todas las estrellas de un solo campo, en una sola captura**:

1. Desenfoque (automático vía el focuser, o manual)
2. Captura de una imagen que contiene varias estrellas repartidas en el campo
3. Detección de cada donut utilizable y cálculo de un vector de deformación por estrella
4. Regresión sobre el conjunto de los vectores para determinar el vector de colimación global
5. Conversión en corrección a aplicar en cada uno de los 3 tornillos
6. Nueva captura, nuevo análisis, bucle continuo hasta la convergencia

### Por qué el campo completo es suficiente

Para una estrella en la posición `P` en la imagen (referencia centrada en la imagen), el vector de deformación observado sigue un modelo lineal:

```
D(P) = C - k·P
```

donde `C` es el vector de colimación buscado (constante, independiente de la posición en el campo) y `k` agrupa el efecto del coma de campo (aberración óptica normal, presente incluso bien colimado, que crece con la distancia al centro y con la apertura del telescopio). Un campo vectorial de este tipo siempre tiene un punto de convergencia único — con algunas estrellas en posiciones de campo diferentes, una regresión permite separar la parte de colimación de la parte de coma, sin necesidad de comparar intra y extra-focal.

Los botones **Go intra**/**Go extra** siguen disponibles para una verificación manual por parte del usuario, pero no es una etapa del flujo automatizado.

### Detección por estrella

Para cada estrella detectada:
- Umbralización adaptativa local y luego refinada individualmente (un umbral global único desfavorecería a las estrellas más débiles del campo)
- Extracción del contorno externo del anillo y de la sombra del secundario (si es visible)
- Un ajuste de círculo por mínimos cuadrados sobre el contorno externo da el centro teórico de la estrella — este método sigue siendo fiable incluso cuando el anillo está muy deformado (creciente, en caso de decolimación importante)
- El vector de deformación es la diferencia entre este centro teórico y el centroide ponderado por intensidad del anillo (sombra excluida)

### Regresión y punto de convergencia

Los vectores de deformación de todas las estrellas detectadas se ajustan por mínimos cuadrados al modelo `D(P) = C - k·P`. El vector `C` obtenido se convierte en arcosegundos (focal y tamaño de píxel del sensor), y luego se proyecta sobre los 3 tornillos de colimación a 120° para obtener un signo y una amplitud por tornillo.

{{% notice style="note" title="Sin calibración en vueltas de tornillo" %}}
La amplitud por tornillo se expresa en arcosegundos, no en vueltas o fracciones de vuelta. La relación entre una desviación óptica y un movimiento físico de tornillo depende del montaje mecánico del secundario (paso común, sensibilidad), propio de cada instrumento — en lugar de inventar una calibración aproximada, el ajuste se hace a ojo: la amplitud y el sentido indicados dan la tendencia, el usuario ajusta y observa la actualización en directo.
{{% /notice %}}

### Mapa de colimación

El análisis de colimación publica su propia imagen anotada (la imagen principal sigue siendo una imagen bruta). Comprende, para cada estrella detectada:
- El contorno externo del donut
- Una flecha de deformación (exagerada para seguir siendo visible), coloreada según la amplitud (verde / naranja / rojo)

Y para el conjunto del campo:
- Un bullseye (anillos discontinuos) centrado en la imagen, que materializa los mismos umbrales de tolerancia que el indicador de calidad
- Una línea que conecta el centro teórico de la imagen con el punto de convergencia calculado

#### Antes de la colimación (ejemplo)

![Overlay de colimación, colimación a corregir](/images/modules/collimator/collimator-ko.png)

#### Después de la colimación (ejemplo)

![Overlay de colimación, colimación correcta](/images/modules/collimator/collimator-ok.png)

### Valores mostrados (colimación)

#### Vector de colimación

| Valor | Descripción |
|---|---|
| **C.x / C.y (px)** | Vector de colimación, en píxeles de imagen |
| **Amplitud (arcsec)** | Amplitud del vector de colimación, convertida en arcosegundos |
| **Punto de convergencia x/y (px)** | Punto de convergencia calculado (donde los vectores de deformación, prolongados, se cruzan) |
| **Quality** | Indicador: verde si la amplitud es baja, naranja en zona intermedia, rojo por encima del umbral alto |

#### Correcciones por tornillo

| Valor | Descripción |
|---|---|
| **Screw 1 / 2 / 3 (turns)** | Amplitud y sentido de corrección por tornillo, expresada en arcosegundos (positivo = apretar) — convención fija a 120°, sin detección automática de la orientación real de los tornillos |

#### Estrellas detectadas

Cuadrícula que lista cada estrella utilizable del último análisis: posición (x, y) y vector de deformación (dx, dy) en píxeles.

## Estados

| Estado | Descripción |
|---|---|
| **Idle** | Módulo inactivo |
| **Shooting** | Adquisición de imagen en curso |
| **Analyzing** | Análisis en curso |

## Posición «home» del focuser

*Set home* memoriza la posición absoluta actual del focuser; *Go home* lleva luego el focuser a ella. La posición **no** se lee al arrancar y **no** se persiste — nada garantiza que el enfoque esté al menos aproximado en ese momento, por lo que debe definirse explícitamente. La propiedad **Focuser home** indica si se ha definido una posición home y su valor; mientras *Set home* no se haya usado, *Go home* se limita a emitir una advertencia y no hace nada.

## Carga desde archivo

El campo **Seleccionar un archivo** permite cargar un archivo FITS desde el disco para analizarlo sin desencadenar una adquisición. El análisis se lanza entonces automáticamente al introducir la ruta, según la *selección de análisis* actual.

## Acciones

| Acción | Descripción |
|---|---|
| **Shoot** | Adquiere una sola imagen y lanza los análisis activados |
| **Loop** | Adquisición y análisis en continuo — se repite automáticamente hasta la detención |
| **Stop** | Detiene el bucle continuo |
| **Reload** | Recarga y reanaliza la última imagen o el último archivo |
| **Set home** | Memoriza la posición actual del focuser como posición de retorno |
| **Go home** | Lleva el focuser a la posición home memorizada (advierte si no se ha definido ninguna) |
| **Go intra** | Desplaza el focuser hacia la posición intra-focal (verificación manual) |
| **Go extra** | Desplaza el focuser hacia la posición extra-focal (verificación manual) |

Los botones **Go intra** / **Go extra** / **Go home** permanecen pulsados durante el desplazamiento del focuser, y se liberan cuando alcanza el objetivo.
