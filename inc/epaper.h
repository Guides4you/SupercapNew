

#include <stdbool.h>
// Hard-coded dimensions of the display
#define WIDTH 152
#define HEIGHT 152

void Epaper_Init(void);

void Epaper_InitQR(uint8_t *buff);
void Epaper_PrintQR(void);

void Epaper_SetBitArray(uint8_t x, bool c, uint8_t *buff);
void Epaper_printconsole(uint8_t *buff, uint8_t size);
void Epaper_Gosleep(void);