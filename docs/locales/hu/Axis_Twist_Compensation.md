# Tengely fordulat kompenzáció

Ez a dokumentum leírja az [axis_twist_compensation] modult.

Some printers may have a small twist in their X rail which can skew the results of a probe attached to the X carriage. This is common in printers with designs like the Prusa MK3, Sovol SV06 etc and is further described under [probe location
bias](Probe_Calibrate.md#location-bias-check). It may result in probe operations such as [Bed Mesh](Bed_Mesh.md), [Screws Tilt Adjust](G-Codes.md#screws_tilt_adjust), [Z Tilt Adjust](G-Codes.md#z_tilt_adjust) etc returning inaccurate representations of the bed.

Ez a modul kézi méréseket használ a felhasználó számára, hogy korrigálja a szonda eredményeit. Vedd figyelembe, hogy ha a tengely jelentősen csavart, akkor határozottan ajánlott először mechanikai eszközöket használni a szoftveres korrekciók alkalmazása előtt.

**Figyelem**: Ez a modul még nem kompatibilis a dokkolható szondákkal, és a szonda csatlakoztatása nélkül próbáld meg megmérni az ágyat, mielőtt használod.

## Overview of compensation usage

> **Tip:** Make sure the [probe X and Y offsets](Config_Reference.md#probe) are correctly set as they greatly influence calibration.

1. After setting up the [axis_twist_compensation] module, perform `AXIS_TWIST_COMPENSATION_CALIBRATE`

* The calibration wizard will prompt you to measure the probe Z offset at a few points along the bed
* The calibration defaults to 3 points but you can use the option `SAMPLE_COUNT=` to use a different number.

1. [Adjust your Z offset](Probe_Calibrate.md#calibrating-probe-z-offset)
1. Perform automatic/probe-based bed tramming operations, such as [Screws Tilt Adjust](G-Codes.md#screws_tilt_adjust), [Z Tilt Adjust](G-Codes.md#z_tilt_adjust) etc
1. Home all axis, then perform a [Bed Mesh](Bed_Mesh.md) if required
1. Perform a test print, followed by any [fine-tuning](Axis_Twist_Compensation.md#fine-tuning) as desired

> **Tip:** Bed temperature and nozzle temperature and size do not seem to have an influence to the calibration process.

## [axis_twist_compensation] setup and commands

Configuration options for [axis_twist_compensation] can be found in the [Configuration Reference](Config_Reference.md#axis_twist_compensation).

Commands for [axis_twist_compensation] can be found in the [G-Codes Reference](G-Codes.md#axis_twist_compensation)
