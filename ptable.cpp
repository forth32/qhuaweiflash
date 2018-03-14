//Процедуры работы с таблицей разделов

#include <QtWidgets>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include "ptable.h"
#include "sio.h"
#include "signver.h"
#include "cpio.h"

int dload_id=-1;
void fdirlist(int np);

//******************************************************
//*  поиск символического имени раздела по его коду
//******************************************************

void  find_pname(unsigned int id,unsigned char* pname, enum parttypes* ptype) {

unsigned int j;
struct  pcl{
  char name[20];
  uint32_t code;
  enum parttypes type;
} pcodes[]={ 
//  --- имя ---       Part ID     тип    
  {"M3Boot",          0x20000    ,part_bin}, 
  {"Ptable"          ,0x10000    ,part_ptable}, 
  {"M3Boot_R11"      ,0x200000   ,part_bin}, 
  {"Ptable_ext_A"    ,0x480000   ,part_ptable},
  {"Ptable_ext_B"    ,0x490000   ,part_ptable},
  {"Fastboot"        ,0x110000   ,part_bin},
  {"Logo"            ,0x130000   ,part_bin},
  {"Kernel"          ,0x30000    ,part_bin},
  {"Kernel_R11"      ,0x90000    ,part_bin},
  {"DTS_R11"         ,0x270000   ,part_bin},
  {"VxWorks"         ,0x40000    ,part_bin},
  {"VxWorks_R11"     ,0x220000   ,part_bin},
  {"M3Image"         ,0x50000    ,part_bin},
  {"M3Image_R11"     ,0x230000   ,part_bin},
  {"DSP"             ,0x60000    ,part_bin},
  {"DSP_R11"         ,0x240000   ,part_bin},
  {"Nvdload"         ,0x70000    ,part_nvram},
  {"Nvdload_R11"     ,0x250000   ,part_nvram},
  {"Nvimg"           ,0x80000    ,part_nvram},
  {"System"          ,0x590000   ,part_cpio},
  {"System"          ,0x100000   ,part_cpio},
  {"APP"             ,0x570000   ,part_cpio}, 
  {"APP"             ,0x5a0000   ,part_cpio}, 
  {"APP_EXT_A"       ,0x450000   ,part_cpio}, 
  {"APP_EXT_B"       ,0x460000   ,part_cpio},
  {"CDROMISO"        ,0xb0000    ,part_iso},
  {"Oeminfo"         ,0xa0000    ,part_oem},
  {"Oeminfo"         ,0x550000   ,part_oem},
  {"Oeminfo"         ,0x510000   ,part_oem},
  {"Oeminfo"         ,0x1a0000   ,part_oem},
  {"WEBUI"           ,0x560000   ,part_cpio},
  {"WEBUI"           ,0x5b0000   ,part_cpio},
  {"Wimaxcfg"        ,0x170000   ,part_bin},
  {"Wimaxcrf"        ,0x180000   ,part_bin},
  {"Userdata"        ,0x190000   ,part_cpio},
  {"Online"          ,0x1b0000   ,part_cpio},
  {"Online"          ,0x5d0000   ,part_cpio},
  {"Online"          ,0x5e0000   ,part_cpio},
  {"Ptable_R1"       ,0x100      ,part_ptable},
  {"Bootloader_R1"   ,0x101      ,part_bin},
  {"Bootrom_R1"      ,0x102      ,part_bin},
  {"VxWorks_R1"      ,0x550103   ,part_bin},
  {"Fastboot_R1"     ,0x104      ,part_bin},
  {"Kernel_R1"       ,0x105      ,part_bin},
  {"System_R1"       ,0x107      ,part_bin},
  {"Nvimage_R1"      ,0x66       ,part_nvram},
  {"WEBUI_R1"        ,0x113      ,part_bin},
  {"APP_R1"          ,0x109      ,part_bin},
  {"HIFI_R11"        ,0x280000   ,part_bin},
  {"Firmwares"       ,0x1e0000   ,part_bin},
  {"Teeos"	     ,0x290000   ,part_bin},
  {0,0}
};

for(j=0;pcodes[j].code != 0;j++) {
  if(pcodes[j].code == id) {
    break;
  }  
}
if (pcodes[j].code != 0) { 
    strcpy((char*)pname,pcodes[j].name); // имя найдено - копируем его в структуру
    *ptype=pcodes[j].type;
}    
else {
    sprintf((char*)pname,"U%08x",id); // имя не найдено - подставляем псевдоимя Uxxxxxxxx в тупоконечном формате
    *ptype=part_bin;
}    
}



//****************************************************
//* Получение описания типа прошивки по коду
//****************************************************
char* fw_description(uint8_t code) {
  
// таблица типов подписей
char* fwtypes[]={
"00-UNKNOWN",        // 0
"01-ONLY_FW",        // 1
"02-ONLY_ISO",       // 2
"03-FW_ISO",         // 3
"04-ONLY_WEBUI",     // 4
"05-FW_WEBUI",       // 5
"06-ISO_WEBUI",      // 6
"07-FW_ISO_WEBUI"    // 7
};  

return fwtypes[code&0x7];  
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
uint8_t* zbuf;
long unsigned int zlen;
int res;

// читаем заголовок в структуру
fread(&table[npart].hd,1,sizeof(pheader),in); // заголовок
//  Ищем символическое имя раздела по таблице 
find_pname(code(npart),table[npart].pname,&table[npart].ptype);

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

// Определение zlib-сжатия

table[npart].zflag=0; 

if ((*(uint16_t*)table[npart].pimage) == 0xda78) {
  table[npart].zflag=table[npart].hd.psize;  // сохраняем сжатый размер 
  zlen=52428800;
  zbuf=(uint8_t*)malloc(zlen);  // буфер в 50М
  // распаковываем образ раздела
  res=uncompress (zbuf, &zlen, table[npart].pimage, table[npart].hd.psize);
  if (res != Z_OK) {
    printf("\n! Ошибка распаковки раздела %s (%02x)\n",table[npart].pname,table[npart].hd.code>>16);
    exit(0);
  }
  // создаем новый буфер образа раздела и копируем в него рапаковынные данные
  free(table[npart].pimage);
  table[npart].pimage=(uint8_t*)malloc(zlen);
  memcpy(table[npart].pimage,zbuf,zlen);
  table[npart].hd.psize=zlen;
  free(zbuf);
  // перерассчитываем контрольные суммы
  calc_crc16(npart);
//   table[npart].hd.crc=crc16((uint8_t*)&table[npart].hd,sizeof(struct pheader));
}

// Загружаем список файлов для файловых разделов
 if (is_cpio(table[npart].pimage)) table[npart].rootdir=load_cpio(table[npart].pimage,table[npart].hd.psize);
//  if (is_cpio(table[npart].pimage)) fdirlist(npart);
// продвигаем счетчик разделов
npart++;
// отъезжаем, если надо, вперед на границу слова
res=ftell(in);
if ((res&3) != 0) fseek(in,(res+4)&(~3),SEEK_SET);
}

//*******************************************************
//* Очистка таблицы разделов
//*******************************************************
void ptable_list::clear() {

int i;
for (i=0;i<npart;i++) {
  free(table[i].csumblock);
  free(table[i].pimage);
  if (table[i].rootdir != 0) delete table[i].rootdir;
}
npart=0;
}


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

// буфер префикса BIN-файла
uint8_t prefix[0x5c];

// Создаем панель диалога прогресс-бара
QWidget* pb=new QWidget();
QVBoxLayout* lm=new QVBoxLayout(pb);

QLabel* label = new QLabel("Поиск и загрузка разделов",pb);
QFont font;
font.setPointSize(14);
font.setBold(true);
font.setWeight(75);
label->setFont(font);
lm->addWidget(label);

QProgressBar* fbar = new QProgressBar(pb);
fbar->setValue(0);
lm->addWidget(fbar);

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
   fbar->setValue(percent);
   QCoreApplication::processEvents();
   oldpercent=percent;
  } 

  if (i == dpattern) break; // найден маркер
}
if (feof(in)) {
  QMessageBox::critical(0,"Ошибка"," В файле не найдены разделы - файл не содержит образа прошивки");
    exit(0);
}  

// текущая позиция в файле должна быть не ближе 0x60 от начала - размер заголовка всего файла
if (ftell(in)<0x60) {
    QMessageBox::critical(0,"Ошибка","Заголовок файла имеет неправильный размер");
    exit(0);
}    
fseek(in,-0x60,SEEK_CUR); // отъезжаем на начало BIN-файла
// вынимаем префикс
fread(prefix,0x5c,1,in);
if (dload_id == -1) {
  // Для первого загруженного файла вынимаем тип прошивки
  dload_id=*((uint32_t*)&prefix[0]);
  // если принудительно dload_id не установлен - выбираем его из заголовка
  if (dload_id > 0xf) {
    QMessageBox::critical(0,"Ошибка","Неверный код типа прошивки (dload_id) в заголовке");
    printf("\n Неверный код типа прошивки (dload_id) в заголовке - %x",dload_id);
    exit(0);
  }
  dload_id&=7; // удаляем бит наличия подписи
  printf("\n Код файла прошивки: %x (%s)",dload_id,fw_description(dload_id));
}

// Поиск разделов
do {
  // обновление индикатора
  percent=ftell(in)*100/filesize;
  if (percent>oldpercent) {
   fbar->setValue(percent);
   QCoreApplication::processEvents();
   oldpercent=percent;
  } 
  if (fread(&i,1,4,in) != 4) break; // конец файла
  if (i != dpattern) break;         // образец не найден - конец цепочки разделов
  fseek(in,-4,SEEK_CUR);            // отъезжаем назад, на начало заголовка
  extract(in);                      // извлекаем раздел
} while(1);

delete fbar;
delete lm;
delete label;
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
free(table[np].pimage);
table[np].pimage=(uint8_t*)malloc(fsize);

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
void ptable_list::save_part(int np,FILE* out,uint8_t zflag) {
 
uint32_t pos,i,cnt;
uint8_t pad=0;  
long unsigned int clen;


struct ptb_t origpt=table[np]; // сохраняем старый описатель раздела
if (zflag) {
  // сжатие образа раздела
  table[np].pimage=(uint8_t*)malloc(table[np].hd.psize+64000);
  clen=table[np].hd.psize+64000;
  compress2(table[np].pimage,&clen,origpt.pimage,origpt.hd.psize,9); 
  table[np].hd.psize=clen;
  calc_crc16(np);
}  
fwrite(hptr(np),1,sizeof(pheader),out);   // заголовок
fwrite(table[np].csumblock,1,crcsize(np),out);  // crc
fwrite(iptr(np),1,psize(np),out);   // тело
// Выравнивание хвоста на границу слова
pos=ftell(out);
if ((pos&3) != 0) {
  cnt=4-(pos%4); // получаем число лишних байт;
  for(i=0;i<cnt;i++) fwrite(&pad,1,1,out);  // записываем нули до границы слова
}  
if (zflag) {
  // чистим буфера
  free(table[np].pimage);
  free(table[np].csumblock);
  table[np]=origpt;
  table[np].csumblock=0;
  calc_crc16(np);
}  

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

//*******************************************************
//* Замена образа раздела
//*******************************************************
void ptable_list::replace(int n, uint8_t* data, uint32_t len) {

free (table[n].pimage);
table[n].pimage=(uint8_t*)malloc(len);
memcpy(table[n].pimage,data,len);
table[n].hd.psize=len;
calc_crc16(n);
}



