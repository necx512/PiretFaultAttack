#ifndef _FOUNDMASTERKEY_H
#define _FOUNDMASTERKEY_H
#include <stdint.h>
void getkeyfromlastroundkey(uint8_t *lastsubkeyround, uint8_t *key);
#endif //_FOUNDMASTERKEY_H
