#ifndef _AESROUND_H
#define _AESROUND_H
#include <stdint.h>
extern uint8_t rsbox[256];
extern uint8_t sbox[256];
unsigned int rev_rotword(unsigned int word);
void oneround(int n/*round from*/,uint8_t *subkeyfrom,uint8_t *subkeyto);
void rev_oneround(int n,uint8_t *subkeyfrom,uint8_t *subkeyto);

#endif //_AESROUND_H
