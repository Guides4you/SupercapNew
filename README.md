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
- **Build tip**: open the `Firmware` folder in Visual Studio Code and follow the project’s build instructions (toolchain, environment variables, and board configuration).

> Firmware: It require Visual code studio and nRF Connect SDK v2.3.0

---

## Images and Programming
- **Target MCU**: **nRF52805**.  
- **Programmers supported**: Segger (J-Flash), Nordic programmer (`nrfjprog`), or other Segger tools.  
- **Common issue**: if the board’s **RESET** pin is not configured as a GPIO, the board may not boot after programming.  
- **Post-programming step (Windows)**: if needed, run the UICR erase command to restore RESET behavior:

nrfjprog --eraseuicr -f NRF52

- **Recommendation**: verify pin configuration in the hardware design before flashing; keep a working Segger or Nordic tool available for recovery.

---

## Hardware
- **Open with**: Altium Designer.  
- **Project outputs**: two production variants are included (with and without ePaper). Each variant contains production files and PDF schematics.  
- **Check**: BOM, gerbers, and PDF schematics in the `Project Outputs` folder before sending to production.

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
- **Default configuration**: `+6+10+1`.

> Default value:  
> +6+10+1

- **Behavior**: after the first boot the configuration file is hidden; every time the TAG is written the device reboots.

---

## Quick checklist before first flash
- Confirm **nRF Connect SDK v2.3.0** is installed and selected in your build environment.  
- Verify **RESET** pin is configured as GPIO in the hardware design.  
- Use a Segger or Nordic programmer and, if necessary, run `nrfjprog --eraseuicr -f NRF52` after programming.  
- Choose the correct hardware variant (with or without ePaper) when preparing production files.

---

## Troubleshooting tips
- **Build fails**: ensure the SDK version is exactly **v2.3.0** and that Visual Studio Code environment variables and toolchain are configured.  
- **Board does not boot after flash**: check RESET pin configuration and run the UICR erase command if required.  
- **ePaper not responding**: confirm you selected the hardware variant with ePaper and verify the ePaper power and control signals in the schematic.

---

## Contributing
- Open a pull request for documentation fixes or improvements.  
- When updating firmware, include the SDK version and exact build steps in the commit message.  
- When updating hardware files, include which variant (with/without ePaper) the change applies to.
