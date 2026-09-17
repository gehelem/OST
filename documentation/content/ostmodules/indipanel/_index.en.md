---
title: IndiPanel
weight: 80
description: Raw control panel for all connected INDI devices
---

## Role

The IndiPanel module is a transparent control panel giving access to all INDI properties of all devices connected to the INDI server. Unlike other OST modules, it does not fulfil a specific astrophotography function: it exposes the full INDI protocol directly within the OST interface.

![IndiPanel module screenshot](/images/modules/indipanel.png)

{{% notice style="note" title="Module with no configuration" %}}
IndiPanel has no configuration file and no parameters to set. Its content is generated entirely on the fly from the devices and properties discovered on the INDI server.
{{% /notice %}}

## How it works

On startup, the module connects to the INDI server and listens for all device and property discovery events. For each INDI property received, a corresponding entry is created automatically in the OST interface:

| INDI type | OST representation |
|---|---|
| Number | Numeric value |
| Switch | On/off button or button group (OneOfMany, AnyOfMany) |
| Text | Text field |
| Light | Status indicator |
| BLOB | Image (JPEG + FITS) |

Properties are organised by INDI device and group, exactly as in any standard INDI client.

## Two-way interaction

- **INDI → OST**: any update to an INDI property (value, state) is immediately reflected in the interface
- **OST → INDI**: any change made by the user is sent back to the corresponding INDI driver (`sendNewSwitch`, `sendNewNumber`, `sendNewText`)

Images (BLOBs) are automatically displayed as soon as a driver sends an image — including from cameras not configured in other modules.

## Use cases

- Direct access to advanced INDI driver properties not exposed by other modules
- Debugging a device or driver
- Controlling an exotic device with no dedicated OST module
- Viewing status messages emitted by INDI drivers

## Devices

No device needs to be configured. The module connects to the INDI server automatically and discovers all available devices.
