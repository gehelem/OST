---
title: Allsky
weight: 90
description: Cámara allsky con keograma, stack, timelapse y datos meteorológicos
---

## Función

El módulo Allsky pilota una cámara gran angular (allsky) en bucle continuo para documentar la noche. En cada imagen, genera una superposición de los datos meteorológicos, construye un keograma y un stack en tiempo real, y produce un timelapse de vídeo al final de la sesión. Las sesiones pasadas se archivan y se pueden consultar.

![Captura de pantalla del módulo Allsky](/images/modules/allsky.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| Cámara | Adquisición de las imágenes allsky |
| GPS | Opcional — posición geográfica para el cálculo de las horas de salida/puesta del sol |
| Estación meteorológica | Opcional — datos meteorológicos mostrados en superposición y en el keograma |

## Parámetros

### Adquisición

| Parámetro | Descripción |
|---|---|
| **Exposición** | Duración de exposición en segundos |
| **Ganancia** | Ganancia de la cámara |
| **Offset** | Offset de la cámara |
| **Retardo (s)** | Intervalo en segundos entre el inicio de dos adquisiciones consecutivas (debe ser superior a la exposición) |

### Planificación

| Parámetro | Descripción |
|---|---|
| **Manual** | Inicia inmediatamente y funciona de forma continua mientras la acción Play esté activa |
| **Hora fija** | Adquisición entre una hora de inicio y una hora de fin (día o noche según el orden de las horas) |
| **Puesta de sol** | Inicia automáticamente al ponerse el sol y se detiene al salir el sol |

Para los modos *Hora fija* y *Puesta de sol*, las horas de la próxima salida y puesta de sol se calculan automáticamente a partir de la posición geográfica configurada (libnova).

### Posición geográfica

La posición geográfica (latitud, longitud) es necesaria para el cálculo de las horas solares. Se puede introducir manualmente o importar desde un dispositivo GPS conectado.

### Exposición/ganancia automática

| Parámetro | Descripción |
|---|---|
| **Activado** | Activa el ajuste automático |
| **Modificar exposición o ganancia** | Elige el parámetro a ajustar |
| **Valor mínimo** | Límite inferior del ajuste |
| **Valor máximo** | Límite superior del ajuste |
| **Medida** | Estadística usada como referencia (Media o Mediana) |
| **Valor objetivo** | Valor objetivo de la estadística |
| **Umbral (%)** | No utilizado actualmente |

El coeficiente de corrección se calcula mediante: `nuevo_valor = valor_actual × (objetivo / medida_actual)`.

### Archivos

| Parámetro | Descripción |
|---|---|
| **Conservar las imágenes individuales** | Si está activado, los JPEG individuales de cada sesión se conservan en los archivos; si no, solo se guardan el keograma, el stack y el timelapse |

## Procesamiento de cada imagen

Para cada imagen adquirida:

1. **Guardado FITS** de la imagen bruta
2. **Superposición** de la fecha/hora y de los datos meteorológicos (temperatura, humedad, presión) en color sobre el JPEG
3. **Stack por máximo**: cada píxel conserva el valor máximo de todas las imágenes de la sesión
4. **Keograma**: una banda vertical de 1 píxel tomada en el centro de cada imagen se une a las anteriores para formar un gráfico temporal de la noche; los datos meteorológicos se representan mediante puntos coloreados (rojo = temperatura, verde = humedad, azul = presión)
5. **Registro SNR**: la relación señal/ruido de la imagen se registra con la marca de tiempo

## Timelapse

Al final de la sesión (detención o fin de la planificación), el módulo genera un timelapse de vídeo (MP4 H.264 a 30 fps) a partir de todos los JPEG de la sesión, vía ffmpeg.

## Archivos

Cada sesión terminada se traslada a una carpeta de archivo con marca de tiempo. Los archivos se pueden consultar directamente desde la interfaz, mostrando para cada sesión:

| Elemento | Descripción |
|---|---|
| **Fecha** | Identificador con marca de tiempo de la sesión |
| **Keograma** | Banda temporal de la noche |
| **Stack** | Imagen apilada (máximo) de la noche |
| **Timelapse** | Vídeo de la noche |

## Datos meteorológicos

Si hay un dispositivo meteorológico INDI configurado, el módulo lee las propiedades `WEATHER_PARAMETERS` para obtener temperatura, humedad y presión. Estos valores se:
- Muestran en tiempo real en la sección **Mediciones**
- Superponen en cada imagen
- Trazan en el keograma
- Registran en un gráfico temporal

## Acciones

| Acción | Descripción |
|---|---|
| **Play** | Inicia el bucle de adquisición según el modo de planificación activo |
| **Pause** | Suspende temporalmente las adquisiciones sin cerrar la sesión |
| **Stop** | Detiene la sesión, genera el timelapse y archiva la sesión |
