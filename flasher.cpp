#include <QtWidgets>

#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <termios.h>
#include <unistd.h>

#include "sio.h"
#include "ptable.h"
#include "flasher.h"

// указатель на класс таблицы разделов
extern ptable_list* ptable;

// указатель на открытый последовательный порт
extern int siofd; // fd для работы с Последовательным портом


//*************************************************
//* Варианты цифровой подписи
//*************************************************

struct sgn{
  uint8_t type;
  uint32_t len;
  char* descr;
}; 

struct sgn signbase[] = {
  {1,2958,"Основная прошивка"},
  {1,2694,"Прошивка E3372s-stick"},
  {2,1110,"Вебинтерфейс+ISO для HLINK-модема"},
  {6,1110,"Вебинтерфейс+ISO для HLINK-модема"},
  {2,846,"ISO (dashboard) для stick-модема"},
  {99,1110,"универсальная"},
  {0,0,0}
};


//****************************************************
//* Определение версии прошивальщика
//*
//*   0 - нет ответа на команду
//*   1 - версия 2.0
//*  -1 - версия не 2.0 
//****************************************************
int dloadversion() {

int res;  
int i;  
QString str;
uint8_t replybuf[1024];

res=atcmd("^DLOADVER?",replybuf);
if (res == 0) return 0; // нет ответа - уже HDLC
if (strncmp((char*)replybuf+2,"2.0",3) == 0) return 1;
for (i=2;i<res;i++) {
  if (replybuf[i] == 0x0d) replybuf[i]=0;
}  
str.sprintf("Неправильная версия монитора прошивки - %s",replybuf+2);
QMessageBox::critical(0,"Ошибка",str);
return -1;
}


//***************************************
//* Конструктор класса прошивальщика
//***************************************
flasher::flasher(QWidget *parent) : QDialog(parent) {

int i;  
QString txt;

setupUi(this);
setWindowFlags (windowFlags() & ~Qt::WindowContextHelpButtonHint); 

for (i=0;signbase[i].len != 0;i++) {
  txt.sprintf("%02i  %5i       %s",signbase[i].type,signbase[i].len,signbase[i].descr);
  signlist->addItem(txt);
}  
}

//***************************************************
//* Завершение работы панели прошивки
//***************************************************
void flasher::leave() {
  
close_port();
reject();
}

//***************************************************
// Отправка команды начала раздела
// 
//  code - 32-битный код раздела
//  size - полный размер записываемого раздела
// 
//*  результат:
//  false - ошибка
//  true - команда принята модемом
//***************************************************
bool dload_start(uint32_t code,uint32_t size) {

uint32_t iolen;  
uint8_t replybuf[4096];
  
static struct __attribute__ ((__packed__)) {
  uint8_t cmd=0x41;
  uint32_t code;
  uint32_t size;
  uint8_t pool[3]={0,0,0};
} cmd_dload_init;

cmd_dload_init.code=htonl(code);
cmd_dload_init.size=htonl(size);
iolen=send_cmd((uint8_t*)&cmd_dload_init,sizeof(cmd_dload_init),replybuf);
if ((iolen == 0) || (replybuf[1] != 2)) return false;
else return true;
}  

//***************************************************
// Отправка блока раздела
// 
//  blk - # блока
//  pimage - адрес начала образа раздела в памяти
// 
//*  результат:
//  false - ошибка
//  true - команда принята модемом
//***************************************************
bool dload_block(uint32_t part, uint32_t blk, uint8_t* pimage) {

uint32_t res,blksize,iolen;
uint8_t replybuf[4096];

static struct __attribute__ ((__packed__)) {
  uint8_t cmd=0x42;
  uint32_t blk;
  uint16_t bsize;
  uint8_t data[4096];
} cmd_dload_block;  
  
blksize=4096; // начальное значение размера блока
res=ptable->psize(part)-blk*4096;  // размер оставшегося куска до конца файла
if (res<4096) blksize=res;  // корректируем размер последнего блока

// номер блока
cmd_dload_block.blk=htonl(blk+1);
// размер блока
cmd_dload_block.bsize=htons(blksize);
// порция данных из образа раздела
memcpy(cmd_dload_block.data,pimage+blk*4096,blksize);
// отсылаем блок в модем
iolen=send_cmd((uint8_t*)&cmd_dload_block,sizeof(cmd_dload_block)-4096+blksize,replybuf); // отсылаем команду

if ((iolen == 0) || (replybuf[1] != 2)) {
  printf("\n sent block:\n");
  dump(&cmd_dload_block,sizeof(cmd_dload_block),0);
  printf("\n\n reply\n");
  dump(replybuf,iolen,0);
  fflush(stdout);
  return false;
}  
else return true;
}

  
//***************************************************
// Завершение записи раздела
// 
//  code - код раздела
//  size - размер раздела
// 
//*  результат:
//  false - ошибка
//  true - команда принята модемом
//***************************************************
bool dload_end(uint32_t code, uint32_t size) {

uint32_t iolen;
uint8_t replybuf[4096];

static struct __attribute__ ((__packed__)) {
  uint8_t cmd=0x43;
  uint32_t size;
  uint8_t garbage[3];
  uint32_t code;
  uint8_t garbage1[12];
} cmd_dload_end;

cmd_dload_end.code=htonl(code);
cmd_dload_end.size=htonl(size);
iolen=send_cmd((uint8_t*)&cmd_dload_end,sizeof(cmd_dload_end),replybuf);
if ((iolen == 0) || (replybuf[1] != 2)) {
  printf("\n sent block:\n");
  dump(&cmd_dload_end,sizeof(cmd_dload_end),0);
  printf("\n\n reply\n");
  dump(replybuf,iolen,0);
  fflush(stdout);
  return false;
}  
else return true;
}  

  
//***************************************
//* Запуск процесса прошивки
//***************************************
int flasher::exec() {
  
char signver[200];
int32_t res,part;
uint32_t iolen,blk,maxblock;
uint8_t replybuf[4096];
QString txt;
unsigned char cmdver=0x0c;

// Варианты ответа модема на HDLC-команды
unsigned char OKrsp[]={0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a};
// ответ на ^signver
unsigned char SVrsp[]={0x0d, 0x0a, 0x30, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a};

// Настройка SIO
if (!open_port())  {
  QMessageBox::critical(0,"Ошибка","Последовательный порт не открывается");
  leave();
  return 0;
}  
  
tcflush(siofd,TCIOFLUSH);  // очистка выходного буфера

res=dloadversion();
if (res == -1) {
  leave(); // неправильная версия прошивочного сервера
  return 0;
}
if (res == 0) {
  QMessageBox::critical(0,"Ошибка","Порт не находится в режиме прошивки");
  leave();
  return 0;
}  

// цифровая подпись
if (dsign->isChecked()) {
 // готовим АТ-команду 
 sprintf(signver,"^SIGNVER=%i,0,778A8D175E602B7B779D9E05C330B5279B0661BF2EED99A20445B366D63DD697,%i",
	 signbase[signlist->currentIndex()].type,
	 signbase[signlist->currentIndex()].len);
 //  Отправлем signver...
 res=atcmd(signver,replybuf);
 if (memcmp(replybuf,SVrsp,sizeof(SVrsp)) != 0) {
  QMessageBox::critical(0,"Ошибка ^signver","Ошибка передачи цифровой подписи");
  leave();
  return 0;
 }
}  

// Входим в HDLC-режим
usleep(100000);
res=atcmd("^DATAMODE",replybuf);
if (res != 6) {
  QMessageBox::critical(0,"Ошибка входа в HDLC","Неправильный ответ на команду ^datamode");
  leave();
  return 0;
}  
if (memcmp(replybuf,OKrsp,6) != 0) {
  QMessageBox::critical(0,"Ошибка входа в HDLC","Команда ^datamode отвергнута модемом");
  leave();
  return 0;
}  


iolen=send_cmd(&cmdver,1,(unsigned char*)replybuf);
if ((iolen == 0)||(replybuf[1] != 0x0d)) {
  QMessageBox::critical(0,"Ошибка протокола HDLC","Невозможно получить версию протокола прошивки");
  leave();
  return 0;
}  

// выводим версию протокола в форму
res=replybuf[2];
replybuf[res+3]=0;
txt.sprintf("%s",replybuf+3);
pversion->setText(txt);
QCoreApplication::processEvents();

// Главный цикл записи разделов
for(part=0;part<ptable->index();part++) {
 // прогресс-бар по разделам 
 totalbar->setValue(part*100/ptable->index());
 // выводим имя раздела в форму
 txt.sprintf("%s",ptable->name(part));
 cpart->setText(txt);

 // команда начала раздела
 if (!dload_start(ptable->code(part),ptable->psize(part))) {
  txt.sprintf("Раздел %s отвергнут",ptable->name(part)); 
  QMessageBox::critical(0,"Ошибка",txt);
  leave();
  return 0;
}  
   
 
maxblock=(ptable->psize(part)+4095)/4096; // число блоков в разделе
// Поблочный цикл передачи образа раздела
for(blk=0;blk<maxblock;blk++) {
 // Прогрессбар блоков
 partbar->setValue(blk*100/maxblock);
 QCoreApplication::processEvents();

 // Отсылаем очередной блок
  if (!dload_block(part,blk,ptable->iptr(part))) {
   txt.sprintf("Блок %i раздела %s отвергнут",blk,ptable->name(part)); 
   QMessageBox::critical(0,"ошибка",txt);
   leave();
   return 0;
 }  
}    

// закрываем раздел
 if (!dload_end(ptable->code(part),ptable->psize(part))) {
  txt.sprintf("Ошибка закрытия раздела %s",ptable->name(part)); 
  QMessageBox::critical(0,"ошибка",txt);
  leave();
  return 0;
 }  
} // конец цикла по разделам

// Выводим модем из HDLC или, если надо, перезагружаем модем.
if (rebootflag->isChecked())   modem_reboot();
else end_hdlc();

close_port();
totalbar->setValue(100);
QCoreApplication::processEvents();

// Завершаем процесс
QMessageBox::information(0,"ОК","Запись завершена без ошибок");
accept();
return 0;
}

 
 
 
 
 