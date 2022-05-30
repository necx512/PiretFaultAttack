#include <stdio.h>
#include "aes.h"
#include <stdlib.h>
#include <assert.h>
#include <time.h>

/*
 0 4 8  12
 1 5 9  13
 2 6 10 14
 3 7 11 15*/
void ShiftRows(uint8_t* state);

int linetofault = 0;
int columntofault = 0;
struct AES_ctx ctx;
uint8_t key[16]={0x54,0x48,0x54,0x5a,0x45,0x64,0x5b,0x45,0x48,0x4a,0x55,0xa4,0x56,0x4c,0x54,0xd9};


void printstate(state_t *state)
{
  for(int line=0;line<4;++line)
  {
    for(int column=0;column<4;++column)
      printf("%02X ",(*state)[column][line]);
    printf("\n");
  }
}

void get4(state_t *cipher, uint8_t *res)
{
	/*int idx = 4*linetofault+columntofault;
	switch(idx)
	{
		case 0:
			res[0] = (*cipher)[0][0];
			res[1] = (*cipher)[3][1];
			res[2] = (*cipher)[2][2];
			res[3] = (*cipher)[1][3];
			break;
		default:
			fprintf(stderr,"not implemented\n");
			exit(EXIT_FAILURE);
	}*/
 switch(columntofault)
 {
   case 0:
     res[0] = (*cipher)[0][0];
     res[1] = (*cipher)[3][1];
     res[2] = (*cipher)[2][2];
     res[3] = (*cipher)[1][3];
     break;

   case 1:
     res[0] = (*cipher)[1][0];
     res[1] = (*cipher)[0][1];
     res[2] = (*cipher)[3][2];
     res[3] = (*cipher)[2][3];
     break;
   
   case 2:
     res[0] = (*cipher)[2][0];
     res[1] = (*cipher)[1][1];
     res[2] = (*cipher)[0][2];
     res[3] = (*cipher)[3][3];
     break;
   
   case 3:
     res[0] = (*cipher)[3][0];
     res[1] = (*cipher)[2][1];
     res[2] = (*cipher)[1][2];
     res[3] = (*cipher)[0][3];
     break;


 }
}
void set4(state_t *cipher, uint8_t *from)
{
 switch(columntofault)
 {
   case 0:
     (*cipher)[0][0] = from[0];
     (*cipher)[3][1] = from[1];
     (*cipher)[2][2] = from[2];
     (*cipher)[1][3] = from[3];
     break;

   case 1:
     (*cipher)[1][0] = from[0];
     (*cipher)[0][1] = from[1];
     (*cipher)[3][2] = from[2];
     (*cipher)[2][3] = from[3];
     break;
   
   case 2:
     (*cipher)[2][0] = from[0];
     (*cipher)[1][1] = from[1];
     (*cipher)[0][2] = from[2];
     (*cipher)[3][3] = from[3];
     break;
   
   case 3:
     (*cipher)[3][0] = from[0];
     (*cipher)[2][1] = from[1];
     (*cipher)[1][2] = from[2];
     (*cipher)[0][3] = from[3];
     break;


 }

}
void xor4(uint8_t* a, uint8_t* b,uint8_t *res)
{
	for(int i=0;i<4;++i)
		res[i] = a[i] ^ b[i];
}
void computeMCK(uint8_t* cipher4, uint8_t* lastkeyguess4,uint8_t *res)
{
	uint8_t resxor[4];
	xor4(cipher4,lastkeyguess4,resxor);
	for(int i=0;i<4;++i)
	  res[i] = rsbox[resxor[i]];
}

uint8_t possibilities[256][4];
void computepossibilities()
{
	state_t state;
	for(int i=0;i<256;++i)
	{
  for(int column=0;column<4;++column)
    for(int line=0;line<4;++line)
      state[column][line] = 0;
		state[columntofault][linetofault] = i;

		MixColumns(&state);
		possibilities[i][0]=state[columntofault][0];
		possibilities[i][1]=state[columntofault][1];
		possibilities[i][2]=state[columntofault][2];
		possibilities[i][3]=state[columntofault][3];

	}
}

int compare2(uint8_t *xor)
{
 /*int count = 0;
	for(int i=0;i<256;++i)
	if(
				xor[0] == possibilities[i][0] && 
				xor[1] == possibilities[i][1])
 {
   count++;
 }
 assert(count==0 || count == 1);*/
  

	for(int i=0;i<256;++i)
		if(
				xor[0] == possibilities[i][0] && 
				xor[1] == possibilities[i][1])
			return i;
	return -1;
}
int compare4(uint8_t *xor,int compare2value)
{
 if(compare2value == -1)
 {
   //just get compare2value by using compare2 and remove the if/else
   for(int i=0;i<256;++i)
   {
	  	if(
	  			xor[0] == possibilities[i][0] && 
	  			xor[1] == possibilities[i][1] &&
	  			xor[2] == possibilities[i][2] &&
	  			xor[3] == possibilities[i][3] 
	  			)
    {
      return 1;
    }
   }
 }
 else
 {
	  	if(
	  			xor[0] == possibilities[compare2value][0] && // can be deleted
	  			xor[1] == possibilities[compare2value][1] && // can be deleted
	  			xor[2] == possibilities[compare2value][2] &&
	  			xor[3] == possibilities[compare2value][3] 
	  			)
    {
      return 1;
    }

 }
	return 0;
}
void print4(char *label,uint8_t *data4)
{
  printf("%s : %02X %02X %02X %02X\n",label,data4[0],data4[1],data4[2],data4[3]);
}

#define NBRMAXCANDIDATES 4096
uint8_t Glistofcandidates[NBRMAXCANDIDATES][4];
int nbrOfGenCandidate=0;
void deleteElm(int idx)
{
  Glistofcandidates[idx][0] = Glistofcandidates[nbrOfGenCandidate-1][0];
  Glistofcandidates[idx][1] = Glistofcandidates[nbrOfGenCandidate-1][1];
  Glistofcandidates[idx][2] = Glistofcandidates[nbrOfGenCandidate-1][2];
  Glistofcandidates[idx][3] = Glistofcandidates[nbrOfGenCandidate-1][3];
  nbrOfGenCandidate--;
}
void generateCiphertext(uint8_t faultedciphertext[4],uint8_t clearciphertext[4]);

void getlistGen()
{
  for(int i=0;i<nbrOfGenCandidate;++i)
  {
    uint8_t *lastkeyguess4 = Glistofcandidates[i];
    print4("Found", lastkeyguess4);
  }
}
void updateGlistofcand(uint8_t currentlistofcandidates[NBRMAXCANDIDATES][4],int nbrOfCandidate)
{
  if(nbrOfGenCandidate == 0)
  {
    for(int i=0;i<nbrOfCandidate;++i)
    {
      Glistofcandidates[i][0] = currentlistofcandidates[i][0];
      Glistofcandidates[i][1] = currentlistofcandidates[i][1];
      Glistofcandidates[i][2] = currentlistofcandidates[i][2];
      Glistofcandidates[i][3] = currentlistofcandidates[i][3];
      nbrOfGenCandidate++;
    }
  }
  else
  {
    for(int i=0;i<nbrOfGenCandidate;++i)
    {
      int j;
      for(j=0;j<nbrOfCandidate;++j)
      {
        if(Glistofcandidates[i][0] == currentlistofcandidates[j][0] && 
            Glistofcandidates[i][1] == currentlistofcandidates[j][1] && 
            Glistofcandidates[i][2] == currentlistofcandidates[j][2] && 
            Glistofcandidates[i][3] == currentlistofcandidates[j][3])
        {
          break;
        }
      }
      if(j == nbrOfCandidate)
      {
        deleteElm(i);
        i--;
      }
    }
  }
}

void generateCiphertext(uint8_t faultedciphertext[4],uint8_t clearciphertext[4])
{
  //////////////////////////////////////////////////////////////////
 uint8_t ptwithoutfault[16];
 uint8_t ptwithfault[16];
 
 FILE *randfile = fopen("/dev/urandom","r");
 for(int i=0;i<16;++i)
 {
	 ptwithoutfault[i] = fgetc(randfile);
	 ptwithfault[i] = ptwithoutfault[i];
 }
 fclose(randfile);



 state_t *faultedciphertextcipher;
 state_t *clearciphertextcipher;

 AES_ECB_encrypt(&ctx, ptwithoutfault,-1,0,linetofault,columntofault,0x04);
 AES_ECB_encrypt(&ctx, ptwithfault,-1,1,linetofault,columntofault,0x04);


 int idxi;
 for(idxi=0;idxi<16 && ptwithoutfault[idxi] == ptwithfault[idxi] ;++idxi);
 assert(idxi != 16);
   

 faultedciphertextcipher = (state_t *)ptwithfault;
 clearciphertextcipher = (state_t *)ptwithoutfault;
 //printstate(faultedciphertextcipher);
 //printf("\n");
 //printstate(clearciphertextcipher);
 
 get4(faultedciphertextcipher,faultedciphertext);
 get4(clearciphertextcipher,clearciphertext);
 /////////////////////////////////////////////////////////////////////////

}

int dotest()
{
 uint8_t currentlistofcandidates[NBRMAXCANDIDATES][4];
 int nbrOfCandidate=0;


 uint8_t faultedciphertext[4];
 uint8_t clearciphertext[4];
 generateCiphertext(faultedciphertext,clearciphertext);


 uint8_t MCKfaultedciphertext[4];
 uint8_t MCKclearciphertext[4];
 uint8_t lastkeyguess4[4];
 uint8_t xor[4];
 for(int lastkeyguess4_0 = 0;lastkeyguess4_0<256;lastkeyguess4_0++)
 {
   lastkeyguess4[0] = lastkeyguess4_0;
   for(int lastkeyguess4_1 = 0;lastkeyguess4_1<256;lastkeyguess4_1++)
   {
     lastkeyguess4[1] = lastkeyguess4_1;
     lastkeyguess4[2] = 0;
     lastkeyguess4[3] = 0;
     
     computeMCK(faultedciphertext, lastkeyguess4,MCKfaultedciphertext);
     computeMCK(clearciphertext, lastkeyguess4,MCKclearciphertext);
     xor4(MCKfaultedciphertext, MCKclearciphertext, xor);

     int compare2value = compare2(xor);
     if(compare2value != -1)
     {

       for(int lastkeyguess4_2 = 0;lastkeyguess4_2<256;lastkeyguess4_2++)
       {
	 lastkeyguess4[2] = lastkeyguess4_2;
         for(int lastkeyguess4_3 = 0;lastkeyguess4_3<256;lastkeyguess4_3++)
         {
	   lastkeyguess4[3] = lastkeyguess4_3;
     
     computeMCK(faultedciphertext, lastkeyguess4,MCKfaultedciphertext);
     computeMCK(clearciphertext, lastkeyguess4,MCKclearciphertext);
     xor4(MCKfaultedciphertext, MCKclearciphertext, xor);



           
           if(compare4(xor,compare2value) ==1)
           {
            assert(nbrOfCandidate < NBRMAXCANDIDATES);
            currentlistofcandidates[nbrOfCandidate][0] = lastkeyguess4[0];
            currentlistofcandidates[nbrOfCandidate][1] = lastkeyguess4[1];
            currentlistofcandidates[nbrOfCandidate][2] = lastkeyguess4[2];
            currentlistofcandidates[nbrOfCandidate][3] = lastkeyguess4[3];
            nbrOfCandidate++;



            //print4("FOUND KEY",lastkeyguess4);
           }
         }
       }
     }
   }
 }
 updateGlistofcand(currentlistofcandidates,nbrOfCandidate);
 return nbrOfGenCandidate;
}
int main()
{
 FILE *randfile = fopen("/dev/urandom","r");
 /*for(int i=0;i<16;++i)
 {
  key[i] = fgetc(randfile);
 }*/

 FILE *masterkeyfile=fopen("/tmp/masterkeyfile","w");
 for(int i=0;i<16;++i)
	 fprintf(masterkeyfile,"%d\n",key[i]);
 fclose(masterkeyfile);
 



 AES_init_ctx(&ctx,key);
 uint8_t subkey4[4];
 //printstate((state_t*)subkey);

 state_t sk;
 char buf[1024];
 for(int i=0;i<4;++i)
 {
   linetofault = fgetc(randfile)%4;
   columntofault = i;
   nbrOfGenCandidate=0;
   //get4((state_t*)subkey,subkey4);
   //sprintf(buf,"subkey4 %d",i);
   //print4(buf,subkey4);
   computepossibilities();
   int p;
   int a=1;
   do
   {
     p=dotest();
   }while(p != 1);
   set4(&sk,Glistofcandidates[0]);
 }
 printstate((state_t*)&sk);
 
 uint8_t *p=(uint8_t*)&sk;
 FILE *subkeyfile=fopen("/tmp/subkeyfile","w");
 for(int i=0;i<16;++i)
	 fprintf(subkeyfile,"%d\n",p[i]);
 fclose(masterkeyfile);
 
 fclose(randfile);
 
}
