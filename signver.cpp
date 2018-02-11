// 
#include <QtWidgets>

// Процедуры обработки цифровых подписей
// 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "sio.h"
#include "ptable.h"
// #include "flasher.h"
// #include "util.h"
// #include "zlib.h"


#define signbaselen 6


// результирующая строка ^signver-команды
uint8_t signver[200];

// Флаг типа прошивки
extern int dflag;

// Параметры текущей цифровой подписи
uint32_t signtype; // тип прошивки
int32_t signlen=-1;  // длина подписи

int32_t serach_sign();

// Хеш открытого ключа для ^signver
char signver_hash[100]="778A8D175E602B7B779D9E05C330B5279B0661BF2EED99A20445B366D63DD697";



  

//***************************************************
//* Отправка цифровой подписи
//***************************************************
int32_t send_signver() {
  
uint32_t res;
// ответ на ^signver
unsigned char SVrsp[]={0x0d, 0x0a, 0x30, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a};
uint8_t replybuf[200];
char message[100];  

signtype=dload_id&0x7;

sprintf((char*)signver,"^SIGNVER=%i,0,%s,%i",signtype,signver_hash,signlen);
res=atcmd((char*)signver,replybuf);
if ( (res<sizeof(SVrsp)) || (memcmp(replybuf,SVrsp,sizeof(SVrsp)) != 0) ) {
   sprintf(message,"Ошибка проверки цифровой сигнатуры - %02x",replybuf[2]);
   QMessageBox::critical(0,"Ошибка",message);
   return -2;
}
return 1;
}

//***************************************************
//* Поиск цифровой подписи в прошивке
//***************************************************
int32_t search_sign() {

int i,j;
uint32_t pt;
uint8_t* imageptr;

// поиск в разделах 0 и 1
for (i=0;i<2;i++) {
  if (ptable->index() == i) break;
  imageptr=ptable->iptr(i)+ptable->psize(i);
  pt=*(uint32_t*)(imageptr-4);
  if (pt == 0xffaaaffa) { 
    // подпись найдена
    signlen=*(uint32_t*)(imageptr-12);
    bzero(signver_hash,100);
    // выделяем хеш открытого ключа
//     printf("\n psize = %08x",
    for(j=0;j<32;j++) {
     sprintf(signver_hash+2*j,"%02X",*(imageptr-signlen+6+j));
    }
    printf("\n hash = %s",signver_hash);
    return signlen;
  }
}
// не найдена
return -1;
}
  