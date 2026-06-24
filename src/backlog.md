# Backlog Bug Firmware

## Priorita alta

1. Overflow/letture fuori limite su URL NFC
   - File: `NfcTag.c`, `main.c`
   - Dettaglio: `NFCGetData()` copia `dummy[8] - 1` byte nel buffer URL senza verificare `URL_BUFFER` e senza garantire il terminatore `\0`.
   - Impatto: possibile corruzione memoria, crash, reboot spurii, advertising BLE corrotto.
   - Riferimenti: `NfcTag.c:62`, `NfcTag.c:65`, `main.c:240`, `main.c:244`, `main.c:304`

2. Overflow stack in `NfcTag_writeBytes`
   - File: `NfcTag.c`
   - Dettaglio: buffer locale fisso `uint8_t buff[20]`, ma `len` non viene validato prima della copia.
   - Impatto: se `len > 18`, scrittura fuori limite sullo stack.
   - Riferimenti: `NfcTag.c:258`, `NfcTag.c:265`, `NfcTag.c:274`

3. Sequenza BLE fragile: HCI usato prima che Bluetooth sia pronto
   - File: `main.c`
   - Dettaglio: `bt_enable(bt_ready)` e' asincrona, ma `set_tx_power()` viene chiamata subito dopo anche fuori da `bt_ready`.
   - Impatto: errore intermittente nella configurazione TX power o comportamento dipendente dal timing di boot.
   - Riferimenti: `main.c:272`, `main.c:274`

4. Possibile payload advertising oltre limite legacy BLE
   - File: `main.c`, `../inc/main.h`
   - Dettaglio: `URL_BUFFER` e' 25 byte; con metadati Eddystone e altre AD structures il payload puo' saturare o superare il limite legacy di 31 byte.
   - Impatto: `bt_le_adv_start()` puo' fallire e il firmware non segnala chiaramente il problema.
   - Riferimenti: `../inc/main.h:9`, `main.c:83`, `main.c:108`, `main.c:244`

## Priorita media

5. Gestione callback GPIO non pulita in interrupt mode
   - File: `main.c`
   - Dettaglio: nel ramo `VOUT_INT_MODE` la callback viene riaggiunta nel loop senza rimozione esplicita.
   - Impatto: possibile comportamento non deterministico o callback multiple se la modalita' viene abilitata.
   - Riferimenti: `main.c:323`, `main.c:324`

6. Error handling assente su API I2C/SPI/PM
   - File: `NfcTag.c`, `GDEW0154M10.c`
   - Dettaglio: diversi return code di `i2c_write_read`, `spi_write`, `pm_device_action_run` non vengono verificati.
   - Impatto: fault hardware o bus error non distinguibili; difficile diagnosi in campo.
   - Riferimenti: `NfcTag.c:34`, `NfcTag.c:40`, `NfcTag.c:251`, `GDEW0154M10.c:105`, `GDEW0154M10.c:156`, `GDEW0154M10.c:174`, `GDEW0154M10.c:196`

## Priorita bassa

7. Include diretto a header interno Zephyr
   - File: `main.c`
   - Dettaglio: viene incluso un header interno del controller Bluetooth Zephyr tramite path relativo.
   - Impatto: fragile rispetto ad aggiornamenti SDK/Zephyr.
   - Riferimento: `main.c:22`

8. Macro multi-istruzione senza wrapper
   - File: `main.c`
   - Dettaglio: `Power_EN()` e `Power_DI()` sono macro multi-istruzione non racchiuse in `do { } while (0)`.
   - Impatto: rischio di bug se usate in contesti condizionali o modificate in futuro.
   - Riferimenti: `main.c:71`, `main.c:77`

9. Valutare persistenza dello stato display aggiornato
   - File: `main.c`
   - Dettaglio: valutare se salvare in memoria persistente Nordic lo stato dell'ultimo contenuto mostrato su e-paper.
   - Obiettivo: evitare refresh display al boot quando i dati NFC non sono cambiati.
   - Impatto: riduzione consumi, tempo di boot e usura/ghosting del display.
   - Note: confrontare almeno URL, power, advertising interval ed eventuale flag `epaper`; definire dove salvare lo stato e come invalidarlo in caso di cambio formato.
