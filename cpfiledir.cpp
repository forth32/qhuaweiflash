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

// имя файла
filename=new char[nsz];
memcpy(filename,iptr+sizeof(cpio_header_t),nsz);

// тело файла
fimage=new char[fsize()];
memcpy(fimage,iptr+sizeof(cpio_header_t)+nsz,fsize());

// printf("\n ---file %s ---\n",fname());
// dump(iptr,sizeof(cpio_header_t)+nsz+20,0);
}

//******************************************************
//* Деструктор класса хранилища файлов
//******************************************************
cpfiledir::~cpfiledir() {

if (subdir != 0) delete subdir;
delete filename;
delete fimage;
}

//*******************************************************
//* Установка нового имени файла
//*******************************************************
void cpfiledir::setfname (char* name) {
  
char* fn=fname();
delete fn;
fn=new char[strlen(name)];
strcpy(fn,name);
}

//*******************************************************
//* Получение размера файла
//*******************************************************
 uint32_t cpfiledir:: fsize() {
  
uint32_t val;
sscanf(phdr->c_filesize,"%8x",&val);
return val;
}

//*******************************************************
//* Установка размера файла
//*******************************************************
void cpfiledir::setfsize(int size) {
  
sprintf(phdr->c_filesize,"%08x",size);
}


//*******************************************************
//* Получение округленной длины имени файла
//*******************************************************
uint32_t cpfiledir:: nsize() {
  
uint32_t val;
sscanf(phdr->c_namesize,"%8x",&val);
val+=sizeof(cpio_header_t); // добавляем размер заголовка
if ((val&3) != 0) val=(val&0xfffffffc)+4; // округляем до 4 байт вверх
return val-sizeof(cpio_header_t);
// return val;
}

//**********************************************************
//* Получение чистого имени файла без предшествующего пути
//**********************************************************
char* cpfiledir::cfname() {
  
char* ptr;

ptr=strrchr(fname(),'/');
if (ptr == 0) return fname();
else return ptr+1;
}

//*******************************************************
//* Получение времени создания файла
//*******************************************************
uint32_t cpfiledir::ftime() {
  
uint32_t val;
sscanf(phdr->c_mtime,"%8x",&val);
return val;
}


//*******************************************************
//* Получение атрибутов файла
//*******************************************************
uint32_t cpfiledir::fmode() {
  
uint32_t val;
sscanf(phdr->c_mode,"%8x",&val);
return val;
}

//*******************************************************
//* Получение группы файла
//*******************************************************
uint32_t cpfiledir::fgid() {
  
uint32_t val;
sscanf(phdr->c_gid,"%8x",&val);
return val;
}

//*******************************************************
//* Получение владельца файла
//*******************************************************
uint32_t cpfiledir::fuid() {
  
uint32_t val;
sscanf(phdr->c_uid,"%8x",&val);
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

if (subdir == 0) {
  for(i=1;i<subdir->count();i++) {
    len += subdir->at(i)->store_cpio(ptr+len);
  }
}
return len;
}

  
  
  
//##############################################################################################################################################
  
//*******************************************************
//* Выделение имени файла из заголовка cpio-архива
//*******************************************************
void extract_filename(uint8_t* iptr, char* filename) {

cpfiledir fd(iptr);
strcpy(filename,fd.fname());
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

// класс, куда загружаются описатели данного файла
cpfiledir* fd=new cpfiledir(iptr);
char filename[256]; // буфер для копии имени файла
char* slptr;
QList<cpfiledir*>* fdir; // подкаталог для поиска остатка имени файла
strncpy(filename,fname,256);      

// Корневой каталог
if ((strlen(filename) == 1) && (filename[1] != '.')) {
//   fd->subdir=dir; // указываем на себя  
   fd->subdir=0; // указываем на себя  
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
    printf("\n file without dir - %s - %s",filename,fname);
    return 0; // не нашли - ошибка структуры, файл без каталога
  }
// загружаем файл в вектор подкаталога    
  return cpio_load_file(iptr,fdir,plen,slptr);
}  
// Это - настоящее конечное имя файла
// для каталога создаем вектор-подкаталог
if ((fd->fmode()&C_ISDIR) != 0) {
   fd->subdir=new QList<cpfiledir*>;
   cpfiledir* upfd=new cpfiledir(iptr);
   upfd->subdir=dir;
   upfd->setfname("..");
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

//*******************************************************
//* Загрузка в память файловой структуры cpio-архива
//*******************************************************
QList<cpfiledir*>* load_cpio(uint8_t* pimage, int len) {

int res;
char filename[512];
// вектор корневого каталога 
QList<cpfiledir*>* root=new QList<cpfiledir*>;

uint8_t* iptr=pimage;  // указатель на текущую позицию в образе раздела

// Цикл разбора cpio-потока
while(iptr < (pimage+len)) {
  // Ищем сигнатуру заголовка очередного файла
  while(1) {
   if (iptr >= (iptr+len)) return 0; // вышли за границу архива, trailer!!! не нашли - архиы битый
   if (is_cpio(iptr)) break; // нашли сигнатуру
   iptr++; // ищем ее дальше
  }  
 extract_filename(iptr,filename);  
 if (strncmp(filename,"TRAILER!!!",10) == 0) break;
 res=cpio_load_file(iptr,root,len,filename);
 if (res == 0) break;
 iptr+=res;
}
qDebug() << "treesize = " << fullsize(root);
return root;
}

