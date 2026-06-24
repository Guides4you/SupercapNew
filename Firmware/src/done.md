# Modifiche eseguite

## Pinout e devicetree

1. `DISP_CS` spostato su `P0.21`
   - Label devicetree: `csgpios`
   - File: `../g4y_beacon_supercap.overlay`
   - Nota: `P0.21` nasce come reset, ma il progetto ha `CONFIG_GPIO_AS_PINRESET=n` in `prj.conf` e `prj_debug.conf`.

2. `GPO` della memoria NFC collegato a `P0.04`
   - Label devicetree: `nfcgpo`
   - File: `../g4y_beacon_supercap.overlay`
   - `VOUTNFC` non e' piu' usato come ingresso GPIO.

## Migrazione NFC a ST25DV04KC

3. Aggiornati indirizzi e registri base del driver NFC
   - User memory I2C: `0x53`
   - System memory I2C: `0x57`
   - Dynamic registers: `0x2000+`
   - File: `NfcTag.h`, `NfcTag.c`

4. Adattata la dimensione memoria per ST25DV04KC
   - `EEPROM_I2C_LENGTH` portato a `512` byte.

5. Disattivate le vecchie logiche M24LR non compatibili
   - Vecchie funzioni password/protection M24LR rese no-op dove non direttamente compatibili.
   - Mantenuta l'API esistente per non rompere il resto del firmware.

6. Aggiunta protezione base sul parsing NDEF URL
   - Il buffer URL viene terminato con `\0`.
   - Limitata la copia rispetto a `URL_BUFFER`.

## GPO e rilevamento scrittura NFC

7. Configurato `GPO` per segnalare scritture RF/NFC in EEPROM
   - `GPO1 = GPO_EN | RF_WRITE_EN`
   - La configurazione usa la password I2C factory-default a zero.
   - La password NFC/RF resta indipendente e puo' essere cambiata in produzione.

8. Gestione interrupt su GPO
   - `P0.04` configurato come input.
   - Interrupt su `GPIO_INT_EDGE_FALLING`, coerente con ST25DV04KC TSSOP open-drain e pull-up esterno.
   - Al trigger viene letto `IT_STS_Dyn` e si procede solo se il bit `RF_WRITE` e' attivo.

9. Logica post-scrittura NFC invariata lato comportamento applicativo
   - Dopo scrittura NFC, il firmware rileggge i dati.
   - Se URL/power/adv/epaper cambiano, esegue `sys_reboot(SYS_REBOOT_COLD)`.
   - L'aggiornamento BLE advertising avviene al boot successivo ricostruendo payload e parametri dai dati NFC.

## Energy harvesting / supercap

10. Configurata Energy Harvesting in modalita' on-demand
    - `EH_MODE = 1`
    - `EH_CTRL_Dyn.EH_EN` disabilitato durante init e prima della rilettura NFC.
    - `EH_CTRL_Dyn.EH_EN` abilitato dopo boot/lettura/display update e dopo gestione evento NFC.

11. Sequenza compatibile con AN4913
    - Energy harvesting disabilitato durante la fase critica di comunicazione NFC/RF.
    - Energy harvesting riabilitato solo dopo completamento lettura/scrittura, per ricaricare il supercap tramite `V_EH`.

## E-paper opzionale

12. Aggiunta define compile-time per presenza e-paper
    - File: `../inc/main.h`
    - Define: `DEVICE_HAS_EPAPER`

13. Disattivate le logiche display quando `DEVICE_HAS_EPAPER == 0`
    - Escluse chiamate a:
      - `Epaper_InitQR()`
      - `Epaper_Init()`
      - `Epaper_PrintQR()`
      - `Epaper_Gosleep()`
    - L'aggiornamento dei dati BLE da NFC resta attivo anche senza e-paper.

## Note di verifica

14. P0.21 come GPIO
    - Il firmware/devicetree lo configura come GPIO.
    - Su hardware gia' programmato va verificato `UICR.PSELRESET`: se contiene ancora `P0.21`, serve cancellazione UICR/recover.

15. Build non eseguita
    - `west`, `cmake` e `ninja` non risultano disponibili nel PATH della shell usata.
