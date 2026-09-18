---
title: Monitor
weight: 75
description: Supervisión e historial de los eventos de sesión
---

## Función

El módulo Monitor registra los eventos emitidos por los módulos esclavos durante una sesión de observación y los presenta en forma gráfica. Permite visualizar a posteriori la calidad del guiado, la evolución del HFR de las imágenes y las intervenciones de enfoque, sobre un eje temporal común.

![Captura de pantalla del módulo Monitor](/images/modules/monitor.png)

## Principio de funcionamiento

El Monitor observa la propiedad `signals` de cada módulo esclavo configurado. Cada vez que un módulo emite un evento (vía `setStateEvent`), el Monitor lo registra con una marca de tiempo y los valores numéricos asociados.

Los eventos se almacenan en memoria durante toda la sesión. Al detenerse, se exportan a un archivo JSON con marca de tiempo bajo `<webroot>/monitor/`.

## Módulos supervisados

| Módulo | Eventos captados |
|---|---|
| Guider | `guideRMS` — RMS total del guiado (arcsec) |
| Guider | `guideSNR` — relación señal/ruido de la detección de estrellas |
| Focus | `focusdone` — HFR final tras el enfoque (píxeles) |
| Secuenciador | `imagehfr` — HFR de la imagen registrada (píxeles) |

## Gráfico de sesión

El gráfico principal muestra el conjunto de la sesión sobre un eje temporal común.

| Curva | Descripción |
|---|---|
| Línea naranja | RMS del guiado (eje izquierdo, arcsec) |
| Puntos azules | HFR de las imágenes adquiridas (eje derecho, píxeles) |
| Triángulos verdes | HFR medido al final del enfoque (eje derecho, píxeles) |
| Línea roja discontinua | SNR del guiado (oculta por defecto, activable vía la leyenda) |

El deslizador doble bajo el gráfico permite restringir la ventana temporal mostrada sin pérdida de los datos en memoria.

## Acciones

| Acción | Descripción |
|---|---|
| **Start** | Inicia una nueva sesión de registro (borra los datos anteriores) |
| **Stop** | Detiene el registro y exporta los datos en JSON |

## Parámetros

### Parms

| Parámetro | Descripción |
|---|---|
| **Auto-start** | Inicia automáticamente el registro al cargar el módulo (guardado en el perfil) |

### Filtrado de la vista

| Parámetro | Descripción |
|---|---|
| **From** | Límite inferior de la visualización (filtra los eventos anteriores a esta hora) |
| **To** | Límite superior de la visualización (filtra los eventos posteriores a esta hora) |
| **Max rows** | Número máximo de líneas conservadas en la vista en directo (por defecto: 200) |

### Módulos supervisados (Slaves)

Cada entrada permite seleccionar la instancia de módulo a supervisar para cada tipo (guider, secuenciador, focus, etc.).

## Persistencia JSON

En cada detención de sesión, se exporta un archivo bajo:

```
<webroot>/monitor/YYYYMMDD-HHmmss.json
```

El archivo contiene los encabezados de columna (`gridheaders`) y la cuadrícula de datos (`grid`) en formato de array de arrays, directamente reutilizable para un análisis externo.

{{% notice style="note" title="Inicio automático" %}}
Con **Auto-start** activado en el perfil, el módulo inicia el registro un segundo después de su carga, sin intervención manual.
{{% /notice %}}
