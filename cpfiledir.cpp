// 
//  cpfiledir - класс для хранения списка файлов, составляющих cpio-архив
// 
#include <QtCore/QVariant>
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "cpio.h"

//******************************************************
//* Конструктор класса хранилища файлов
//******************************************************
cpfiledir::cpfiledir(uint8_t* iptr) {
  
phdr=new cpio_header_t;

memcpy(phdr,iptr,sizeof(cpio_header_t)); // копируем себе заголовок
int nsz=nsize();
volatile int fsz=fsize();

// имя файла
filename=new char[nsz];
memcpy(filename,iptr+sizeof(cpio_header_t),nsz);

// тело файла
if (fsz != 0) fimage=new char[fsz];
memcpy(fimage,iptr+sizeof(cpio_header_t)+nsz,fsz);

}

//******************************************************
//* Деструктор класса хранилища файлов
//******************************************************
cpfiledir::~cpfiledir() {

if ((subdir != 0) && !updirflag) {  // если это не ссылка на родительский каталог
    // удаляем вектор подкаталога со всем содержимым
    qDeleteAll(*subdir);
    subdir->clear();
    delete subdir;
}  

delete [] filename;
if (fimage != 0) delete [] fimage;
delete phdr;

}

//*******************************************************
//* Установка нового имени файла
//*******************************************************
void cpfiledir::setfname (char* name) {
  
int len;

delete [] filename;
len=strlen(name)+1;
filename=new char[len];
strcpy(filename,name);
setfsize(len);
  
}

//*******************************************************
//* Получение размера файла
//*******************************************************
 uint32_t cpfiledir:: fsize() {
  
uint32_t val;
char vstr[9];
bzero(vstr,9);
strncpy(vstr,phdr->c_filesize,8);
val=strtoul(vstr,0,16);
return val;
}

//*******************************************************
//* Установка размера файла
//*******************************************************
void cpfiledir::setfsize(int size) {
  
char str[10];  

sprintf(str,"%08x",size);  
memcpy(phdr->c_filesize,str,8);

}



//*******************************************************
//* Получение округленной длины имени файла
//*******************************************************
uint32_t cpfiledir:: nsize() {
  
uint32_t val;
char vstr[9];

bzero(vstr,9);
strncpy(vstr,phdr->c_namesize,8);
val=strtoul(vstr,0,16);
val+=sizeof(cpio_header_t); // добавляем размер заголовка
if ((val&3) != 0) val=(val&0xfffffffc)+4; // округляем до 4 байт вверх
return val-sizeof(cpio_header_t);
}

//**********************************************************
//* Получение чистого имени файла без предшествующего пути
//**********************************************************
char* cpfiledir::cfname() {
  
char* ptr;

ptr=strrchr(filename,'/');
if (ptr == 0) return filename;
else return ptr+1;
}

//*******************************************************
//* Получение времени создания файла
//*******************************************************
uint32_t cpfiledir::ftime() {
  
uint32_t val;
char vstr[9];

bzero(vstr,9);
strncpy(vstr,phdr->c_mtime,8);
val=strtoul(vstr,0,16);
return val;
}


//*******************************************************
//* Получение атрибутов файла
//*******************************************************
uint32_t cpfiledir::fmode() {
  
uint32_t val;
char vstr[9];

bzero(vstr,9);
strncpy(vstr,phdr->c_mode,8);
val=strtoul(vstr,0,16);
return val;
}

//*******************************************************
//* Получение группы файла
//*******************************************************
uint32_t cpfiledir::fgid() {
  
uint32_t val;
char vstr[9];

bzero(vstr,9);
strncpy(vstr,phdr->c_gid,8);
val=strtoul(vstr,0,16);
return val;
}

//*******************************************************
//* Получение владельца файла
//*******************************************************
uint32_t cpfiledir::fuid() {
  
uint32_t val;
char vstr[9];

bzero(vstr,9);
strncpy(vstr,phdr->c_uid,8);
val=strtoul(vstr,0,16);
return val;
}

//*****************************************************************
//* Получение полного размера файла или всех файлов в подкаталоге
//*****************************************************************
uint32_t cpfiledir::treesize() {

uint32_t sum=0;
int i;

if (subdir == 0) return totalsize(); // для некаталогов
for(i=1;i<subdir->count();i++) {
  sum+=subdir->at(i)->treesize();
}
return totalsize()+sum;
}

//*****************************************************************
//* Перепаковка текущего каталога в cpio-архив
//*  ptr - буфер для сохранения данных
//*  возвращает размер полученного архива
//*****************************************************************
uint32_t cpfiledir::store_cpio(uint8_t* ptr) {

int i;
uint32_t len=sizeof(cpio_header_t);
uint32_t size,nlen;

// сохраняем заголовок
memcpy(ptr,phdr,len);
// имя файла
size=nsize();
memcpy(ptr+len,filename,size);
len+=size;
//тело файла
size=fsize();
memcpy(ptr+len,fimage,size);
len+=size;
// округляемся до 4 байт
if ((len&3) != 0) {
  nlen=(len&0xfffffffc)+4;
  bzero(ptr+len,nlen-len);
  len=nlen;
}

// обрабатываем подкаталоги

if (subdir != 0) {
  for(i=1;i<subdir->count();i++) {
    len += subdir->at(i)->store_cpio(ptr+len);
  }
}

return len;
}

//*******************************************************
//* Замена тела файла
//*******************************************************
void cpfiledir::replace_data(uint8_t* pdata, uint32_t len) {

delete [] fimage;
fimage=new char[len];
memcpy(fimage,pdata,len);

setfsize(len);
}
  
  
//##############################################################################################################################################
  
//*******************************************************
//* Выделение имени файла из заголовка cpio-архива
//*******************************************************
void extract_filename(uint8_t* iptr, char* filename) {

strcpy(filename,(char*)(iptr+sizeof(cpio_header_t)));
}

//*******************************************************
//* Поиск подкаталога по имени
//*******************************************************
QList<cpfiledir*>* find_dir(char* name, QList<cpfiledir*>* updir) {
  
int i;
char* fn;
for (i=0;i<updir->size();i++) {
  fn=updir->at(i)->cfname();
  if (strcmp(fn, name) == 0) return updir->at(i)->subdir;
}
return 0;
}

//*******************************************************
//* Поиск файла по имени в указанном каталоге
//*******************************************************
int find_file(QString name, QList<cpfiledir*>* dir) {

int i;
char* fn;
for (i=0;i<dir->size();i++) {
  fn=dir->at(i)->cfname();
  if (name == fn) return i;
}
return -1;
}
  
  
//*******************************************************
//* Определение наличия cpio-потока
//*******************************************************
int is_cpio(uint8_t* ptr) {

if (strncmp((char*)ptr,"070701",6) == 0) {
  return 1;
}  
else {
  return 0;	
}
}

//*******************************************************
//* Загрузка в вектор единичного файла
//*
//* iptr - ссылка на заголовок файла в cpio-потоке
//* dir - указатель на каталог, к которому относится файл
//* plen - общая длина области памяти, хранящей архив
//* filename - имя файла без предшествующего пути.
//*******************************************************
uint32_t cpio_load_file(uint8_t* iptr, QList<cpfiledir*>* dir, int plen, char* fname) {

char* dfname=(char*)"..";  
QString str;
// класс, куда загружаются описатели данного файла
cpfiledir* fd;
char filename[256]; // буфер для копии имени файла
char* slptr;
QList<cpfiledir*>* fdir; // подкаталог для поиска остатка имени файла
strncpy(filename,fname,256);      

// Корневой каталог
if ((strlen(filename) == 1) && (filename[1] != '.')) {
  fd=new cpfiledir(iptr);
  fd->subdir=0; // нет подкаталога  
  dir->append(fd);
  return fd->totalsize();
}
// Проверяем наличие пути к файлу      
slptr=strchr(filename,'/');

if (slptr != 0) {
  // это еще не конечное имя файла, а элемент пути
  *slptr=0; // разрезаем имя файла на верхний каталог и остальное
  slptr++;  // теперь slptr показывает на остаток имени файла
  fdir=find_dir(filename, dir); // ищем подкаталог в текущем каталоге
  if (fdir == 0) {
    str.sprintf("В потоке обнаружен файл без каталога - %s",fname);
    QMessageBox::critical(0,"Ошибка CPIO",str);
    return 0; // не нашли - ошибка структуры, файл без каталога
  }
// загружаем файл в вектор подкаталога   
  return cpio_load_file(iptr,fdir,plen,slptr);
}  
// Это - настоящее конечное имя файла
// для каталога создаем вектор-подкаталог
fd=new cpfiledir(iptr);
if ((fd->fmode()&C_ISDIR) != 0) {
   // вектор подкаталога
   fd->subdir=new QList<cpfiledir*>;
   // указатель на каталог верзнего уровня (то есть вот этот)
   cpfiledir* upfd=new cpfiledir(iptr);
   upfd->subdir=dir;
   // имя файла для него - ".."
   upfd->setfname(dfname);
   upfd->updirflag=true;  // признак ссылки на каталог верхнего уровня
   // добавляем эту запись первой в вектор подкаталога
   fd->subdir->append(upfd);
}  
dir->append(fd);
return fd->totalsize();
}

//*******************************************************
//* Подсчет полного размера загруженного архива
//*******************************************************
uint32_t fullsize(QList<cpfiledir*>* root) {
  
uint32_t sum=0;
int i;

for(i=0;i<root->count();i++) {
  sum+=root->at(i)->treesize();
}  
return sum;
}

