#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "foundmasterkey.h"
#include "tree.h"
#include "aesround.h"

void getkeyfromlastroundkeyv1(uint8_t *lastsubkeyround, uint8_t *key);

struct list *getlast(struct list *Ri1,int idxi)
{
  struct listentry *current = Ri1->firstelm;
  while(current != NULL)
  {
    uint8_t res[16];
    rev_oneround(idxi,current->e->lastkey,res);
    for(int i=0;i<16;++i)
      current->e->lastkey[i] = res[i];
    current = current->next;
  }

  return generateExtensionFromList(Ri1);

}
void getkeyfromlastroundkeyv1(uint8_t *lastsubkeyround, uint8_t *key)
{
  struct list *R10 = generateExtensionFromList(generateSimpleElmList(lastsubkeyround));
  printlist("R10",R10);
  struct list *R9  = getlast(R10,9);
  printlist("R9",R9);
  struct list *R8  = getlast(R9,8);
  printlist("R8",R8);
  struct list *R7  = getlast(R8,7);
  printlist("R7",R7);
  struct list *R6  = getlast(R7,6);
  printlist("R6",R6);
  struct list *R5  = getlast(R6,5);
  printlist("R5",R5);
  struct list *R4  = getlast(R5,4);
  printlist("R4",R4);
  struct list *R3  = getlast(R4,3);
  printlist("R3",R3);
  struct list *R2  = getlast(R3,2);
  printlist("R2",R2);
  struct list *R1  = getlast(R2,1);
  printlist("R1",R1);
  struct list *R0  = getlast(R1,0);
  printlist("R0",R0);

}
int mainv1()
{
  sbox[0]=0x41;
  /*printf("->%02X\n",rsbox[0x41]);
  uint8_t test[16]={0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  struct list *R = generateExtensionFromList(generateSimpleElmList(test));
   printlist("R",R);
   printf("------------\n");
   exit(0);*/



	char line[1024];
 uint8_t masterkey[16];
 FILE *masterkeyfile = fopen("/tmp/masterkeyfile","r");
 assert(masterkeyfile != NULL);
 for(int i=0;i<16;++i)
 {
   fgets(line,16,masterkeyfile);
   masterkey[i] = atoi(line);
 }
 fclose(masterkeyfile);

	FILE *subkeyfile = fopen("/tmp/subkeyfile","r");
	assert(subkeyfile!=NULL);
	uint8_t lastsubkeyround[16];
	uint8_t key[16];
	for(int i=0;i<16;++i)
	{
		fgets(line,16,subkeyfile);
		lastsubkeyround[i] = atoi(line);
		printf("%02X:",lastsubkeyround[i]);
	}
	printf("\n\n\n\n");
	getkeyfromlastroundkeyv1(lastsubkeyround,key);
/* int OK=1;
	for(int i=0;i<16;++i)
	{
		printf("%02X:",key[i]);
  if(key[i] != masterkey[i])
    OK=0;
	}
	printf("\n");
	fclose(subkeyfile);
 if(OK == 1)
   printf("-OK-\n");
 else
   printf("-FAIL-\n");*/
 return 0;
}

void getkeyfromlastroundkeyv0(uint8_t *lastsubkeyround, uint8_t *key)
{
	uint8_t subkeyround[11][16];
	for(int j=0;j<16;++j)
          subkeyround[10][j] = lastsubkeyround[j];
 
 /*printf("R%02d : ",10);
	for(int j=0;j<16;++j)
	  printf("%02X ",subkeyround[10][j]);
 printf("\n");*/

 for(int i=9;i>=0;--i)
 {
          rev_oneround(i,subkeyround[i+1],subkeyround[i]);
          /*printf("R%02d : ",i);
	         for(int j=0;j<16;++j)
	           printf("%02X ",subkeyround[i][j]);
          printf("\n");*/
 }
	for(int j=0;j<16;++j)
	  key[j] = subkeyround[0][j];
}
int mainv0()
{
  sbox[0]=0x41;
	char line[1024];
 uint8_t masterkey[16];
 FILE *masterkeyfile = fopen("/tmp/masterkeyfile","r");
 assert(masterkeyfile != NULL);
 for(int i=0;i<16;++i)
 {
   fgets(line,16,masterkeyfile);
   masterkey[i] = atoi(line);
 }
 fclose(masterkeyfile);

	FILE *subkeyfile = fopen("/tmp/subkeyfile","r");
	assert(subkeyfile!=NULL);
	uint8_t lastsubkeyround[16];
	uint8_t key[16];
	for(int i=0;i<16;++i)
	{
		fgets(line,16,subkeyfile);
		lastsubkeyround[i] = atoi(line);
		//printf("%02X:",lastsubkeyround[i]);
	}
	//printf("\n\n\n\n");
	getkeyfromlastroundkeyv0(lastsubkeyround,key);
 int OK=1;
	for(int i=0;i<16;++i)
	{
		printf("%02X:",key[i]);
  if(key[i] != masterkey[i])
    OK=0;
	}
	printf("\n");
	fclose(subkeyfile);
 /*if(OK == 1)
   printf("-OK-\n");
 else
   printf("-FAIL-\n");*/
 return OK;
}
  
int main()
{
  return mainv0();
}
