#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <termios.h>
#include <unistd.h>

#include "sio.h"
#include "usbloader.h"

// указатель на открытый последовательный порт
extern int siofd; // fd для работы с Последовательным портом


//*************************************************
//* Рассчет контрольной суммы командного пакета
//*************************************************
void csum(unsigned char* buf, uint32_t len) {

unsigned  int i,c,csum=0;

unsigned int cconst[]={0,0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF};

for (i=0;i<len;i++) {
  c=(buf[i]&0xff);
  csum=((csum<<4)&0xffff)^cconst[(c>>4)^(csum>>12)];
  csum=((csum<<4)&0xffff)^cconst[(c&0xf)^(csum>>12)];
}  
buf[len]=(csum>>8)&0xff;
buf[len+1]=csum&0xff;
  
}

//*************************************************
//*   Отсылка командного пакета модему
//*************************************************
int sendcmd(void* srcbuf, int len) {

unsigned char replybuf[1024];
unsigned char cmdbuf[2048];
unsigned int replylen;

// локальная копия командного буфера
memcpy(cmdbuf,srcbuf,len);

// добавляем в нее контрольную сумму
csum(cmdbuf,len);

// отсылка команды
write(siofd,cmdbuf,len+2);  
tcdrain(siofd);

// читаем ответ
replylen=read(siofd,replybuf,1024);

if (replylen == 0) return 0;     // пустой ответ
if (replybuf[0] == 0xaa) return 1; // правильный ответ
return 0;
}

//*************************************
//* Поиск linux-ядра в образе раздела
//*************************************
int locate_kernel(uint8_t* pbuf, uint32_t size) {
  
int off;

for(off=(size-8);off>0;off--) {
  if (strncmp((char*)(pbuf+off),"ANDROID!",8) == 0) return off;
}
return 0;
}

//*********************************************
//* Поиск таблицы разделов в загрузчике 
//*********************************************
uint32_t find_ptable(uint8_t* buf, uint32_t size) {

// сигнатура заголовка таблицы  
const uint8_t headmagic[16]={0x70, 0x54, 0x61, 0x62, 0x6c, 0x65, 0x48, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};  
uint32_t off;

for(off=0;off<(size-16);off+=4) {
  if (memcmp(buf+off,headmagic,16) == 0)   return off;
}
return 0;
}



//***************************************
//* Конструктор класса прошивальщика
//***************************************
usbloader::usbloader(QWidget *parent) : QDialog(parent) {

QString txt;

setupUi(this);
setWindowFlags (windowFlags() & ~Qt::WindowContextHelpButtonHint); 
 
}

//***************************************
//* Чтение замещающей таблицы разделов
//***************************************
void usbloader::ptbrowse() {

static QString fn="";
uint8_t ptbuf[0x800];
FILE* in;
uint32_t fsize;
uint32_t ptoff;

fn=QFileDialog::getOpenFileName(this,"Выбор файла таблицы разделов",fn,"ptable (*.bin);;All files (*.*)");
in=fopen(fn.toLocal8Bit(),"r");
if (in == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка открытия файла");
  return;
}

// загружаем файл в буфер
fsize=fread(ptbuf,1,0x800,in);
fclose(in);
if (fsize != 0x800) {
  QMessageBox::critical(0,"Ошибка","Слишком короткий файл");
  return;
}  
if (strncmp((char*)ptbuf,"pTableHead",10) != 0) {
  QMessageBox::critical(0,"Ошибка","Файл не является таблицей разделов");
  return;
}  

// ищем таблицу разделов внутри загрузчика
ptoff=find_ptable(pbuf[1], part[1].size);
if (ptoff == 0) {
  QMessageBox::critical(0,"Ошибка","В загрузчике на найдена встроенная таблица разделов");
  return;
}  
// замещаем таблицу разделов
memcpy(pbuf[1]+ptoff,ptbuf,0x800);
ptfilename->setText(fn);
}



//***************************************
//* Чтение и разбор загрузчика
//***************************************
void usbloader::browse() {

static QString fn="";
uint32_t i;
FILE* in;
uint32_t fsize;

fn=QFileDialog::getOpenFileName(this,"Выбор файла загрузчика",fn,"usbloader (*.bin);;All files (*.*)");
in=fopen(fn.toLocal8Bit(),"r");
if (in == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка открытия файла");
  return;
}

// Прверяем сигнатуру usloader
fread(&i,1,4,in);
if (i != 0x20000) {
  QMessageBox::critical(0,"Ошибка","Файл не является загрузчиком usbloader");
  leave();
  return;
}  

// читаем заголовок загрузчика
fseek(in,36,SEEK_SET); // начало каталога компонентов в файле
fread(&part,sizeof(part),1,in);

// Ищем конец каталога компонентов
for (i=0;i<5;i++) {
  if (part[i].lmode == 0) break;
}
numparts=i;

// Загружаем компоненты в память
for(i=0;i<numparts;i++) {
 // встаем на начало образа компонента
 fseek(in,part[i].offset,SEEK_SET);
 // освобождаем предыдущий распределенный буфер
 if (pbuf[i] != 0) {
   free(pbuf[i]);
   pbuf[i]=0;
 }  
 // читаем в буфер весь компонент
 pbuf[i]=(uint8_t*)malloc(part[i].size);
 fsize=fread(pbuf[i],1,part[i].size,in);
 if (part[i].size != fsize) {
      QMessageBox::critical(0,"Ошибка","Неожиданный конец файла");
      leave();
      return;
 }  
}

fclose(in);
// выводим в форму имя загруженного файла
filename->setText(fn);
okbutton->setEnabled(true);
fbflag->setEnabled(true);
ptselector->setEnabled(true);
}

//***************************************
// fastboot-патч
//***************************************
void usbloader::fastboot_only() {

int koff;  // смещение до ANDROID-заголовка

koff=locate_kernel(pbuf[1],part[1].size);
if (koff != 0) {
      *(pbuf[1]+koff)=0x55; // патч сигнатуры
      part[1].size=koff+8; // обрезаем раздел до начала ядра
}
else  QMessageBox::critical(0,"Ошибка"," В загрузчике нет ANDROID-компонента - fastboot-загрузка невозможна");
}


//***************************************************
//* Завершение работы панели прошивки
//***************************************************
void usbloader::leave() {
  
close_port();
reject();
}


//***************************************
//* Отправка заголовка компонента
//***************************************
int start_part(uint32_t size,uint32_t adr,uint8_t lmode) {
  
struct __attribute__ ((__packed__)) {
  uint8_t cmd[3]={0xfe,0, 0xff};
  uint8_t lmode;
  uint32_t size;
  uint32_t adr;
}  cmdhead;

cmdhead.size=htonl(size);
cmdhead.adr=htonl(adr);
cmdhead.lmode=lmode;
  
return sendcmd(&cmdhead,sizeof(cmdhead));
}  
  
    
//***************************************
//* Отправка пакета данных
//***************************************
int send_data_packet(uint32_t pktcount, uint8_t* databuf, uint32_t datasize) {

// образ пакета
struct __attribute__ ((__packed__)) {
 uint8_t cmd=0xda; 
 uint8_t count;
 uint8_t rcount;
 uint8_t data[2048];
} cmddata;
  
cmddata.count=pktcount&0xff;
cmddata.rcount=(~pktcount)&0xff;
memcpy(cmddata.data,databuf,datasize);

return sendcmd(&cmddata,datasize+3);
}  


//***************************************
//* Закрытие потока данных компонента
//***************************************
int close_part(uint32_t pktcount) {

struct __attribute__ ((__packed__)) {
 uint8_t cmd=0xed;
 uint8_t count;
 uint8_t rcount;
} cmdeod; 
  
// Фрмируем пакет конца данных
cmdeod.count=pktcount&0xff;;
cmdeod.rcount=(~pktcount)&0xff;

return sendcmd(&cmdeod,sizeof(cmdeod));
}

//***************************************
//* Запуск загрузки
//***************************************
int usbloader::exec() {

uint32_t bl,datasize,pktcount;
uint32_t adr;
uint8_t c;

// Настройка SIO
if (!open_port())  {
  QMessageBox::critical(0,"Ошибка","Последовательный порт не открывается");
  leave();
  return 0;
}  


// Проверяем загрузочный порт
c=0;
write(siofd,"A",1);   // отправляем произвольный байт в порт
bl=read(siofd,&c,1);
// ответ должен быть U (0x55)
if (c != 0x55) {
  QMessageBox::critical(0,"Ошибка","Последовательный порт не находится в режиме USB Boot");
  leave();
  return 0;
}  

// делаем fastboot-патч
if (fbflag->isChecked()) fastboot_only();

// главный цикл загрузки - загружаем все блоки, найденные в заголовке

for(bl=0;bl<numparts;bl++) {

 // Прогрессбар компонентов
 totalbar->setValue(bl*100/numparts);
 QCoreApplication::processEvents();
  
 // стартовый пакет  
 if (!start_part(part[bl].size,part[bl].adr,part[bl].lmode)) {
   QMessageBox::critical(0,"Ошибка","Модем отверг заголовок компонента");
   leave();
   return 0;
 }  

  // Цикл поблочной загрузки данных
  datasize=1024;
  pktcount=1;
  for(adr=0;adr<part[bl].size;adr+=1024) {
    // проверка на последний блок компонента
    if ((adr+1024)>=part[bl].size) datasize=part[bl].size-adr; 
     
    // обновляем прогрессбар блоков 
    partbar->setValue(adr*100/part[bl].size);
    QCoreApplication::processEvents();
    
    if (!send_data_packet(pktcount++,(uint8_t*)(pbuf[bl]+adr),datasize)) {
      QMessageBox::critical(0,"Ошибка","Модем отверг пакет данных");
      leave();
      return 0;
    }  
  }


  if (!close_part(pktcount)) {
      QMessageBox::critical(0,"Ошибка","Модем отверг команду окончания компонента");
      leave();
      return 0;
    }  
} 

totalbar->setValue(100);
QCoreApplication::processEvents();
      
QMessageBox::critical(0,"ОК","Загрузка окончена");

accept();
return 0;
}

  