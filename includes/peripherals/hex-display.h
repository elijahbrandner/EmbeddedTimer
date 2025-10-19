#ifndef HEX_DISPLAY_H
#define HEX_DISPLAY_H

//* Init & Close
int init_hex0_hex3(void);
int init_hex4_hex5(void);
int close_hex0_hex3(void);
int close_hex4_hex5(void);

//* Write
int hex_display_write(int display, int value);
int hex_display_clear(int display);
void hex_display_clear_all(void);

#endif // HEX_DISPLAY_H