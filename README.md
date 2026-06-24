# Supercap New

A concise guide to the **Supercap New** project: repository layout, build requirements, programming instructions, hardware notes, and TAG configuration.

---

## Folder structure
- **Firmware** — firmware source code.  
- **Images** — binary firmware files.  
- **Hardware** — Altium project files and production outputs.

---

## Firmware
- **Required tools**: Visual Studio Code and **nRF Connect SDK v2.3.0**.  
- **Important**: builds will fail with other SDK versions; use exactly **v2.3.0** for reliable results.  
---

## Images and Programming
- **Target MCU**: **nRF52805**.  
- **Programmers supported**: Segger (J-Flash), Nordic programmer (`nrfjprog`), or other Segger tools.  
- **Common issue**: if the board’s **RESET** pin is not configured as a GPIO, the board may not boot after programming.  
- **Post-programming step (Windows)**: if needed, run the UICR erase command to restore RESET behavior:

`nrfjprog --eraseuicr -f NRF52`

---

## Hardware
- **Open with**: Altium Designer.  
- **Project outputs**: two production variants are included (with and without ePaper). Each variant contains production files and PDF schematics.  

---

## TAG configuration
- **Purpose**: TAGs are programmed with a **URI string** (web address) and an optional text configuration file.  
- **Configuration format**: `+pwr+adv+ep`  
- **pwr** — Bluetooth transmit power (integer).
  
| pwr value | nRF52 TX Power Setting               |
|-----------|--------------------------------------|
| 0         | -40 dBm                              |
| 1         | -20 dBm                              |
| 2         | -16 dBm                              |
| 3         | -12 dBm                              |
| 4         | -8  dBm                              |
| 5         | -4  dBm                              |
| 6         | 0   dBm                              |
| 7         | +3  dBm                              |
| 8         | +4  dBm                              | 
- **adv** — advertising time in **seconds × 10** (integer).  
- **ep** — ePaper flag: `1` = enabled, `0` = disabled.  
- **Default configuration**:
> +6+10+1

- **Behavior**: after the first boot the configuration file is hidden; every time the TAG is written the device reboots.
