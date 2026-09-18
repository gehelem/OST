---
title: IndiPanel
weight: 80
description: Panel de control bruto de todos los dispositivos INDI conectados
---

## Función

El módulo IndiPanel es un panel de control transparente que da acceso a todas las propiedades INDI de todos los dispositivos conectados al servidor INDI. A diferencia de los demás módulos OST, no cumple una función astrofotográfica específica: expone directamente la totalidad del protocolo INDI en la interfaz OST.

![Captura de pantalla del módulo IndiPanel](/images/modules/indipanel.png)

{{% notice style="note" title="Módulo sin configuración" %}}
IndiPanel no tiene archivo de configuración ni parámetros que definir. Su contenido se genera enteramente al vuelo a partir de los dispositivos y propiedades descubiertos en el servidor INDI.
{{% /notice %}}

## Funcionamiento

Desde el arranque, el módulo se conecta al servidor INDI y escucha todos los eventos de descubrimiento de dispositivos y propiedades. Para cada propiedad INDI recibida, se crea automáticamente una entrada correspondiente en la interfaz OST:

| Tipo INDI | Representación OST |
|---|---|
| Number | Valor numérico |
| Switch | Botón on/off o grupo de botones (OneOfMany, AnyOfMany) |
| Text | Campo de texto |
| Light | Indicador de estado |
| BLOB | Imagen (JPEG + FITS) |

Las propiedades se organizan por dispositivo INDI y por grupo, exactamente como en cualquier cliente INDI estándar.

## Interacción bidireccional

- **INDI → OST**: cualquier actualización de una propiedad INDI (valor, estado) se refleja inmediatamente en la interfaz
- **OST → INDI**: cualquier modificación realizada por el usuario se envía al controlador INDI correspondiente (`sendNewSwitch`, `sendNewNumber`, `sendNewText`)

Las imágenes (BLOBs) se muestran automáticamente en cuanto un controlador envía una imagen — incluso desde cámaras no configuradas en otros módulos.

## Casos de uso

- Acceso directo a las propiedades avanzadas de un controlador INDI no expuestas por los demás módulos
- Depuración de un dispositivo o de un controlador
- Control de un dispositivo exótico sin módulo OST dedicado
- Consulta de los mensajes de estado emitidos por los controladores INDI

## Dispositivos

No hay ningún dispositivo que configurar. El módulo se conecta automáticamente al servidor INDI y descubre todos los dispositivos presentes.
