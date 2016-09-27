// Процедуры работы с таблицей разделов

#include <QtWidgets>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ptable.h"
#include "ui_findparts.h"
#include "sio.h"

//******************************************************
//*  поиск символического имени раздела по его коду
//******************************************************

void  find_pname(unsigned int id,unsigned char* pname) {

unsigned int j;
struct  pcl{
  char name[20];
  uint32_t code;
} pcodes[]={ 
  {"M3Boot",0x20000}, 
  {"M3Boot-ptable",0x10000}, 
  {"M3Boot_R11",0x200000}, 
  {"Ptable",0x10000},
  {"Fastboot",0x110000},
  {"Logo",0x130000},
  {"Kernel",0x30000},
  {"Kernel_R11",0x90000},
  {"VxWorks",0x40000},
  {"VxWorks_R11",0x220000},
  {"M3Image",0x50000},
  {"M3Image_R11",0x230000},
  {"DSP",0x60000},
  {"DSP_R11",0x240000},
  {"Nvdload",0x70000},
  {"Nvdload_R11",0x250000},
  {"Nvimg",0x80000},
  {"System",0x590000},
  {"System",0x100000},
  {"APP",0x570000}, 
  {"APP",0x5a0000}, 
  {"Oeminfo",0xa0000},
  {"CDROMISO",0xb0000},
  {"Oeminfo",0x550000},
  {"Oeminfo",0x510000},
  {"Oeminfo",0x1a0000},
  {"WEBUI",0x560000},
  {"WEBUI",0x5b0000},
  {"Wimaxcfg",0x170000},
  {"Wimaxcrf",0x180000},
  {"Userdata",0x190000},
  {"Online",0x1b0000},
  {"Online",0x5d0000},
  {"Online",0x5e0000},
  {"Ptable_R1",0x100},
  {"Bootloader_R1",0x101},
  {"Bootrom_R1",0x102},
  {"VxWorks_R1",0x550103},
  {"Fastboot_R1",0104},
  {"Kernel_R1",0x105},
  {"System_r1",0x107},
  {"Nvimage_r1",0x66},
  {0,0}
};

for(j=0;pcodes[j].code != 0;j++) {
  if(pcodes[j].code == id) break;
}
if (pcodes[j].code != 0) strcpy((char*)pname,pcodes[j].name); // имя найдено - копируем его в структуру
else sprintf((char*)pname,"U%08x",id); // имя не найдено - подставляем псевдоимя Uxxxxxxxx в тупоконечном формате
}

//*******************************************************************
//* Извлечение раздела из файла и добавление его в таблицу разделов
//*
//  in - входной файл прошивки
//  Позиция в файле соответствует началу заголовка раздела
//*******************************************************************
void ptable_list::extract(FILE* in)  {

uint16_t hcrc,crc;
QString str;
uint16_t* crcblock;
uint32_t crcblocksize;

// читаем заголовок в структуру
fread(&table[npart].hd,1,sizeof(pheader),in); // заголовок
//  Ищем символическое имя раздела по таблице 
find_pname(code(npart),table[npart].pname);

// загружаем блок контрольных сумм
table[npart].csumblock=0;  // пока блок не создан
crcblock=(uint16_t*)malloc(crcsize(npart)); // выделяем временную память под загружаемый блок
crcblocksize=crcsize(npart);
fread(crcblock,1,crcblocksize,in);

// загружаем образ раздела
table[npart].pimage=(uint8_t*)malloc(psize(npart));
fread(table[npart].pimage,1,psize(npart),in);

// проверяем CRC заголовка
hcrc=table[npart].hd.crc;
table[npart].hd.crc=0;  // старая CRC в рассчете не учитывается
crc=crc16((uint8_t*)&table[npart].hd,sizeof(pheader));
if (crc != hcrc) {
    str.sprintf("Раздел %s (%02x) - ошибка контрольной суммы заголовка",table[npart].pname,code(npart)>>16);
    QMessageBox::warning(0,"Ошибка CRC",str);
}  
table[npart].hd.crc=crc;  // восстанавливаем CRC

// вычисляем и проверяем CRC раздела
calc_crc16(npart);
if (crcblocksize != crcsize(npart)) {
    str.sprintf("Раздел %s (%02x) - неправильный размер блока контрольных сумм",table[npart].pname,code(npart)>>16);
    QMessageBox::warning(0,"Ошибка CRC",str);
}  
  
else if (memcmp(crcblock,table[npart].csumblock,crcblocksize) != 0) {
    str.sprintf("Раздел %s (%02x) - неправильная блочная контрольная сумма",table[npart].pname,code(npart)>>16);
    QMessageBox::warning(0,"Ошибка CRC",str);
}  
  
free(crcblock);
// продвигаем счетчик разделов
npart++;
// отъезжаем немного назад
fseek(in,-16,SEEK_CUR);
}

//*******************************************************
//* Очистка таблицы разделов
//*******************************************************
void ptable_list::clear() {

int i;
for (i=0;i<npart;i++) {
  free(table[i].csumblock);
  free(table[i].pimage);
}
npart=0;
}


//*******************************************************
// Класс прогрессбара разбора разделов
//*******************************************************
  
class pfindbar: public QDialog, public Ui_pfindbar {
  
public:
   pfindbar(QWidget *parent = 0) {
     setupUi(this);
   }
   ~pfindbar() {
   }  
public slots:

void setbar(int val) {
    fbar->setValue(val);
}
  
};  
  
//*******************************************************
//*  Поиск разделов в файле прошивки
//* 
//* возвращает число найденных разделов
//*******************************************************
void ptable_list::findparts(FILE* in) {


const unsigned int dpattern=0xa55aaa55; // Маркер начала заголовка раздела   
unsigned int i;
uint8_t percent,oldpercent=0;
uint32_t filesize;

pfindbar* pb=new pfindbar;
pb->show();
// получаем размер файла
fseek(in,0,SEEK_END);
filesize=ftell(in);
rewind(in);

// поиск начала цепочки разделов в файле
while (fread(&i,1,4,in) == 4) {
  // обновление индикатора
  percent=ftell(in)*100/filesize;
  if (percent>oldpercent) {
   pb->fbar->setValue(percent);
   QCoreApplication::processEvents();
   oldpercent=percent;
  } 

  if (i == dpattern) break; // найден маркер
}
if (feof(in)) {
  QMessageBox::critical(0,"Ошибка"," В файле не найдены разделы - файл не содержит образа прошивки");
  return;
}  

// отъезжаем на начало маркера
fseek(in,-4,SEEK_CUR); 

// Поиск разделов
while (fread(&i,1,4,in) == 4) {
  
  // обновление индикатора
  percent=ftell(in)*100/filesize;
  if (percent>oldpercent) {
   pb->fbar->setValue(percent);
   QCoreApplication::processEvents();
   oldpercent=percent;
  } 
  
  if (i != dpattern) {
    fseek(in,-3,SEEK_CUR);  // в предположении, что маркер может начинаться с любого байта
    continue; 
  }  
  fseek(in,-4,SEEK_CUR); // встаем на начало заголовка
  extract(in); // извлекаем раздел из файла  
}
delete pb;  
}

//*******************************************************
//*  Замена образа раздела на содержимое файла 
//*******************************************************
void ptable_list::loadimage(int np, FILE* in) {

uint32_t fsize;

// определяем размер файла
fseek(in,0,SEEK_END);
fsize=ftell(in);
rewind(in);

// выделяем память под новый раздел
realloc(table[np].pimage,fsize);

// читаем новый образ раздела
fread(table[np].pimage,1,fsize,in);

// корректируем размер раздела в заголовке
table[np].hd.psize=fsize;
// перевычисляем блочную crc16
calc_crc16(np);
fclose(in);
}

//*******************************************************
//* Запись полного образа раздела в файл
//*******************************************************
void ptable_list::save_part(int np,FILE* out) {
  

fwrite(hptr(np),1,sizeof(pheader),out);   // заголовок
fwrite(table[np].csumblock,1,crcsize(np),out);  // crc
fwrite(iptr(np),1,psize(np),out);   // тело
}

//*******************************************************
//*  Вычисление блочной контрольной суммы заголовка
//*******************************************************
void ptable_list::calc_hd_crc16(int n) { 

table[n].hd.crc=0;   
table[n].hd.crc=crc16((uint8_t*)hptr(n),sizeof(pheader));   
}


//*******************************************************
//*  Вычисление блочной контрольной суммы раздела 
//*******************************************************
void ptable_list::calc_crc16(int n) {
  
uint32_t csize; // размер блока сумм в 16-битных словах
uint16_t* csblock;  // указатель на создаваемый блок
uint32_t off,len;
uint32_t i;
uint32_t blocksize=table[n].hd.blocksize; // размер блока, охватываемого суммой

// определяем размер и создаем блок
csize=psize(n)/blocksize;
if (psize(n)%blocksize != 0) csize++; // Это если размер образа не кратен blocksize
csblock=(uint16_t*)malloc(csize*2);

// цикл вычисления сумм
for (i=0;i<csize;i++) {
 off=i*blocksize; // смещение до текущего блока 
 len=blocksize;
 if ((psize(n)-off)<blocksize) len=psize(n)-off; // для последнего неполного блока 
 csblock[i]=crc16(iptr(n)+off,len);
} 
// вписываем параметры в заголовок
if (table[n].csumblock != 0) free(table[n].csumblock); // уничтожаем старый блок, если он был
table[n].csumblock=csblock;
table[n].hd.hdsize=csize*2+sizeof(pheader);
// перевычисляем CRC заголовка
calc_hd_crc16(n);
  
}

  
//*******************************************************
//* Удаление раздела
//*******************************************************
void ptable_list::delpart(int n) {

int i;
// освобождаем занятую память
free(table[n].csumblock);
free(table[n].pimage);
// сдвигаем цепочку разделов вверх
for (i=n;i<index()-1;i++)  table[i]=table[i+1];
// уменьшаем счетчик разделов
npart--;
}  
  

//*******************************************************
//* Перемещение раздела вверх
//*******************************************************
void ptable_list::moveup(int n) {

struct ptb_t tmp;

if (n == 0) return;
tmp=table[n-1];
table[n-1]=table[n];
table[n]=tmp;
}

//*******************************************************
//* Перемещение раздела вниз
//*******************************************************
void ptable_list::movedown(int n) {

struct ptb_t tmp;

if (n == (npart-1)) return;
tmp=table[n+1];
table[n+1]=table[n];
table[n]=tmp;
}

