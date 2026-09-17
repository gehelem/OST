---
title: Monitor
weight: 75
description: Session event monitoring and logging
---

## Role

The Monitor module records events emitted by slave modules during an observation session and presents them as graphs. It lets you visualise, on a shared time axis, guiding quality, image HFR evolution, and focus interventions.

![Monitor module screenshot](/images/modules/monitor.png)

## How it works

The Monitor watches the `signals` property of each configured slave module. Whenever a module emits an event (via `setStateEvent`), the Monitor records it with a timestamp and the associated numeric values.

Events are kept in memory for the entire session. On stop, they are exported to a timestamped JSON file under `<webroot>/monitor/`.

## Monitored modules

| Module | Captured events |
|---|---|
| Guider | `guideRMS` — total guiding RMS (arcsec) |
| Guider | `guideSNR` — star detection signal-to-noise ratio |
| Focus | `focusdone` — final HFR after focusing (pixels) |
| Sequencer | `imagehfr` — HFR of the recorded image (pixels) |

## Session graph

The main graph displays the entire session on a shared time axis.

| Curve | Description |
|---|---|
| Orange line | Guiding RMS (left axis, arcsec) |
| Blue dots | HFR of acquired images (right axis, pixels) |
| Green triangles | HFR measured at the end of a focus run (right axis, pixels) |
| Red dashed line | Guiding SNR (hidden by default, toggle via the legend) |

The double slider below the graph restricts the displayed time window without discarding in-memory data.

## Actions

| Action | Description |
|---|---|
| **Start** | Starts a new recording session (clears previous data) |
| **Stop** | Stops recording and exports data to JSON |

## Parameters

### Parms

| Parameter | Description |
|---|---|
| **Auto-start** | Automatically starts recording when the module loads (saved in the profile) |

### View filter

| Parameter | Description |
|---|---|
| **From** | Lower display bound (hides events before this time) |
| **To** | Upper display bound (hides events after this time) |
| **Max rows** | Maximum number of rows kept in the live view (default: 200) |

### Monitored modules (Slaves)

Each entry selects the module instance to watch for a given type (guider, sequencer, focus, etc.).

## JSON persistence

On each session stop, a file is exported to:

```
<webroot>/monitor/YYYYMMDD-HHmmss.json
```

The file contains column headers (`gridheaders`) and the data grid (`grid`) as an array of arrays, directly usable for external analysis.

{{% notice style="note" title="Auto-start" %}}
With **Auto-start** enabled in the profile, the module begins recording one second after loading, without any manual action.
{{% /notice %}}
