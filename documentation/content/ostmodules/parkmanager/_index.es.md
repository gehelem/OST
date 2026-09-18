---
title: Park Manager
weight: 70
description: Automatización de la apertura y el cierre del observatorio
---

## Función

El módulo Park Manager orquesta la apertura y el cierre completo del observatorio: apertura de la persiana de la cúpula, desestacionamiento de la montura, inicio del seguimiento — y en orden inverso para el cierre. Puede funcionar en modo manual (acción puntual) o en modo automático controlado por reglas horarias y meteorológicas.

![Captura de pantalla del módulo Park Manager](/images/modules/parkmanager.png)

## Dispositivos requeridos

| Dispositivo | Función |
|---|---|
| Cúpula | Apertura y cierre de la persiana, estacionamiento |
| Montura | Desestacionamiento, seguimiento, retorno a home, estacionamiento |
| Estación meteorológica | Supervisión de las condiciones para el modo automático (opcional) |

## Parámetros

### Reglas meteorológicas

| Parámetro | Descripción |
|---|---|
| **Must be OK** | Si está activado, la apertura se rechaza si el tiempo no está en verde. Durante la sesión, una degradación meteorológica desencadena automáticamente el cierre |
| **Disabled** | Desactiva la supervisión meteorológica (se ignora la estación meteorológica) |

### Módulos esclavos

| Parámetro | Descripción |
|---|---|
| **Planner instance** | Nombre de la instancia del módulo Planner a detener al iniciar la secuencia de cierre |

### Reglas horarias

Estas reglas definen la ventana de apertura automática en modo **Automate**. Son mutuamente excluyentes.

| Parámetro | Descripción |
|---|---|
| **Fixed time** | Ventana horaria fija definida por una hora de inicio y una hora de fin |
| **Fixed time: start** | Hora de inicio de la ventana fija |
| **Fixed time: end** | Hora de fin de la ventana fija (puede ser el día siguiente — gestión del paso de medianoche) |
| **Dusk and dawn** | Ventana automática desde la puesta hasta la salida del sol (calculada a partir de las coordenadas GPS) |
| **Anytime** | Apertura permanente: el módulo abre en cuanto está en modo Auto y permanece abierto |

## Secuencias

### Secuencia de apertura

1. Verificación meteorológica (si la regla *Must be OK* está activa)
2. Apertura de la persiana de la cúpula
3. Espera de confirmación de apertura de la persiana
4. Desestacionamiento de la montura
5. Espera de confirmación de desestacionamiento
6. Inicio del seguimiento sideral
7. Entrada en supervisión (*Open Monitoring*)

### Secuencia de cierre

1. Corte de la refrigeración en todos los dispositivos conectados que expongan un switch `CCD_COOLER` (no solo la cámara usada por el Secuenciador), y solicitud de detención a la instancia del Planner
2. Detención del seguimiento sideral
3. Envío de la montura a la posición home
4. Espera de llegada a la posición home
5. Estacionamiento de la montura
6. Espera de confirmación de estacionamiento
7. Cierre de la persiana de la cúpula
8. Espera de confirmación de cierre de la persiana

{{% notice style="warning" title="Orden de cierre" %}}
La montura siempre se estaciona **antes** del cierre de la cúpula. Nunca interrumpir manualmente la secuencia entre estas dos etapas.
{{% /notice %}}

{{% notice style="tip" title="Propagación de la detención" %}}
Detener el Planner interrumpe a su vez el Secuenciador (que interrumpe él mismo al Guider y al Focuser) así como el Navigator — ver la documentación [Planner](../planner/). Esto garantiza que toda la cadena de adquisición se detiene correctamente antes del cierre físico del observatorio.
{{% /notice %}}

### Parada de emergencia

La acción **Abort** envía simultáneamente una orden de detención a la montura (`TELESCOPE_ABORT_MOTION`) y a la cúpula (`DOME_ABORT_MOTION`), y luego vuelve al modo Idle.

## Modo automático

En modo **Automate**, el módulo evalúa cada 2 segundos si el observatorio debe abrirse o cerrarse según las reglas horarias y meteorológicas configuradas.

{{< mermaid >}}
flowchart TB
    ATTENTE([Espera]) -->|"ventana activa y tiempo OK"| OD

    subgraph OUV ["Secuencia de apertura"]
        direction LR
        OD[Abre cúpula] --> OW[Espera apertura] --> UD[Desestaciona montura] --> UW[Espera desestacionamiento] --> TR[Inicia seguimiento]
    end

    TR --> SURV([Supervisión])
    SURV -->|"ventana terminada o tiempo degradado"| ST

    subgraph FER ["Secuencia de cierre"]
        direction LR
        ST[Detiene seguimiento] --> GH[Goto home] --> WH[Espera home] --> PM[Estaciona montura] --> WP[Espera estacionamiento] --> CD[Cierra cúpula]
    end

    CD --> ATTENTE
    ATTENTE -->|Abort| IDLE([Idle])
    SURV -->|Abort| IDLE
{{< /mermaid >}}

| Situación | Acción |
|---|---|
| Ventana abierta + observatorio cerrado | Lanzamiento de la secuencia de apertura |
| Ventana cerrada + observatorio abierto | Lanzamiento de la secuencia de cierre |
| Tiempo degradado durante la sesión | Lanzamiento de la secuencia de cierre |
| Fin de secuencia | Vuelta a espera — se conserva el modo Auto |

{{% notice style="tip" title="Persistencia del modo Auto" %}}
Tras una secuencia de apertura o de cierre, el módulo permanece en modo **Auto** y sigue supervisando las condiciones. No es necesario reactivarlo manualmente.
{{% /notice %}}

## Acciones

| Acción | Descripción |
|---|---|
| **Open all** | Lanza la secuencia de apertura completa |
| **Close all** | Lanza la secuencia de cierre completa |
| **Automate** | Activa el modo automático controlado por las reglas |
| **Abort motion** | Parada de emergencia de todos los movimientos |
| **Idle** | Vuelve al modo inactivo sin acción |

## Estado mostrado

### Montura

| Valor | Descripción |
|---|---|
| **Mount RA** | Ascensión recta actual |
| **Mount DEC** | Declinación actual |
| **Mount is tracking** | Seguimiento sideral activo |
| **Mount is parked** | Montura en posición de estacionamiento |
| **Mount is at home** | Montura en posición home |

### Cúpula

| Valor | Descripción |
|---|---|
| **Dome is parked** | Cúpula en posición de estacionamiento |
| **Dome shutter is closed** | Persiana cerrada |

### Tiempo y eventos

| Valor | Descripción |
|---|---|
| **Global** | Estado global del tiempo (verde / naranja / rojo) |
| **Next sunset** | Hora de la próxima puesta de sol |
| **Next sunrise** | Hora de la próxima salida del sol |
