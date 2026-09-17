---
title: Allsky
weight: 90
description: Allsky camera with keogram, stacking, timelapse, and weather data
---

## Role

The Allsky module drives a wide-angle (allsky) camera in a continuous loop to document the night. For each frame, it generates a weather data overlay, builds a real-time keogram and stack, then produces a timelapse video at the end of the session. Past sessions are archived and browsable.

![Allsky module screenshot](/images/modules/allsky.png)

## Required devices

| Device | Role |
|---|---|
| Camera | Allsky image acquisition |
| GPS | Optional — geographic position for sunrise/sunset time calculation |
| Weather station | Optional — weather data overlaid on images and in the keogram |

## Parameters

### Acquisition

| Parameter | Description |
|---|---|
| **Exposure** | Exposure duration in seconds |
| **Gain** | Camera gain |
| **Offset** | Camera offset |
| **Delay (s)** | Interval in seconds between the start of two consecutive acquisitions (must be greater than exposure) |

### Schedule

| Parameter | Description |
|---|---|
| **Manual** | Starts immediately and runs continuously while the Play action is active |
| **Fixed time** | Acquisition between a start and an end time (daytime or nighttime depending on the order of the times) |
| **Sunset** | Starts automatically at sunset and stops at sunrise |

For *Fixed time* and *Sunset* modes, the next sunrise and sunset times are computed automatically from the configured geographic position (libnova).

### Geographic position

The geographic position (latitude, longitude) is required for solar time calculation. It can be entered manually or imported from a connected GPS device.

### Auto exposure/gain

| Parameter | Description |
|---|---|
| **Enable** | Enables automatic adjustment |
| **Change exposure or gain** | Selects which parameter to adjust |
| **Minimum value** | Lower bound of the adjustment |
| **Maximum value** | Upper bound of the adjustment |
| **Measure** | Statistic used as reference (Mean or Median) |
| **Target value** | Target value for the statistic |
| **Threshold (%)** | Not currently used |

The correction coefficient is computed as: `new_value = current_value × (target / current_measure)`.

### Archives

| Parameter | Description |
|---|---|
| **Keep individual images** | If enabled, individual JPEGs for each session are kept in the archive; otherwise only the keogram, stack, and timelapse are retained |

## Per-image processing

For each acquired image:

1. **FITS save** of the raw image
2. **Overlay** of date/time and weather data (temperature, humidity, pressure) in colour on the JPEG
3. **Maximum stack**: each pixel retains the maximum value across all images of the session
4. **Keogram**: a 1-pixel-wide vertical strip taken from the centre of each frame is appended to the previous ones to form a time-chart of the night; weather data is represented by coloured dots (red = temperature, green = humidity, blue = pressure)
5. **SNR log**: the signal-to-noise ratio of the image is recorded with a timestamp

## Timelapse

At the end of a session (manual stop or end of schedule), the module generates an MP4 H.264 timelapse video at 30 fps from all session JPEGs, using ffmpeg.

## Archives

Each completed session is moved to a timestamped archive folder. Archives are browsable directly from the interface, showing for each session:

| Item | Description |
|---|---|
| **Date** | Timestamped session identifier |
| **Keogram** | Night time-strip |
| **Stack** | Maximum-stacked image for the night |
| **Timelapse** | Night video |

## Weather data

If an INDI weather device is configured, the module reads `WEATHER_PARAMETERS` properties to retrieve temperature, humidity, and pressure. These values are:
- Displayed in real time in the **Measures** section
- Overlaid on each image
- Plotted in the keogram
- Recorded in a time-series graph

## Actions

| Action | Description |
|---|---|
| **Play** | Starts the acquisition loop according to the active schedule mode |
| **Pause** | Temporarily suspends acquisitions without closing the session |
| **Stop** | Stops the session, generates the timelapse, and archives the session |
