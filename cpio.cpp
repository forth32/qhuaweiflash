// 
//  Сохранение файла прошивки на диск
// 
#include <QtCore/QVariant>
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "MainWindow.h"
#include "ptable.h"
#include "cpio.h"
void dump(void* mem,int len,long base);


//******************************************************
//* Конструктор класса хранилища файлов
//******************************************************
cpfiledir::cpfiledir(uint8_t* iptr) {
  
phdr=(cpio_header_t*)iptr;
}

//******************************************************
//* Деструктор класса хранилища файлов
//******************************************************
cpfiledir::~cpfiledir() {

if (subdir != 0) delete subdir;
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
//* Получение длины имени файла
//*******************************************************
uint32_t cpfiledir:: nsize() {
  
uint32_t val;
sscanf(phdr->c_namesize,"%8x",&val);
return val;
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
uint32_t res;
cpfiledir* fd=new cpfiledir(iptr);
char filename[256]; // буфер для копии имени файла
char* slptr;
QList<cpfiledir*>* fdir; // подкаталог для поиска остатка имени файла
strncpy(filename,fname,256);      

// пропускаем файл по имени "."
if ((strlen(filename) == 1) && (filename[1] != '.')) {
  res=fd->totalsize();
  delete fd;
  return res;
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
}  
dir->append(fd);
return fd->totalsize();
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
return root;
}

QList<cpfiledir*>* currentdir;

//*******************************************************
//*  Формирование списка файлов
//*******************************************************
void MainWindow::cpio_create_list(QList<cpfiledir*>* dir) {

QTableWidgetItem* item;
QString str;
QStringList(plst);

int i,j;
time_t ctime;
char tstr[100];
uint32_t fm;
char modestr[10];

cpioedit=new QTableWidget(0,6,centralwidget);

plst << "Name" << "size" << "Date" << "Mode" << "GID" << "UID"; //
cpioedit->setHorizontalHeaderLabels(plst);

currentdir=dir;

for (i=0;i<dir->count();i++) {
  cpioedit->setRowCount(cpioedit->rowCount()+1);
  
  // имя файла
  str=dir->at(i)->cfname();
  item=new QTableWidgetItem(str);
  if (dir->at(i)->subdir == 0) item->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon)));
  else item->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon)));
  cpioedit->setItem(i,0,item);

  // размер файла
  if (dir->at(i)->subdir == 0) {
   str.sprintf("%i",dir->at(i)->fsize());
   item=new QTableWidgetItem(str);
   cpioedit->setItem(i,1,item);
  } 
  
  // дата-время
  ctime=dir->at(i)->ftime();
  strftime(tstr,100,"%d-%b-%y  %H:%M",localtime(&ctime));
  str=tstr;
  item=new QTableWidgetItem(str);
  cpioedit->setItem(i,2,item);
  
  // атрибуты доступа
  fm=dir->at(i)->fmode();
  strcpy(modestr,"rwxrwxrwx");
  for (j=0;j<9;j++) {
    if (((fm>>j)&1) == 0) modestr[8-j]='-';
  }  
  str=modestr;
  item=new QTableWidgetItem(str);
  cpioedit->setItem(i,3,item);
  
  // gid
  str.sprintf("%i",dir->at(i)->fgid());
  item=new QTableWidgetItem(str);
  cpioedit->setItem(i,4,item);
  
  // uid
  str.sprintf("%i",dir->at(i)->fuid());
  item=new QTableWidgetItem(str);
  cpioedit->setItem(i,5,item);
} 
  cpioedit->resizeColumnsToContents();
  cpioedit->setShowGrid(false);
  cpioedit->setColumnWidth(0, 210);
  cpioedit->setColumnWidth(1, 100);

  connect(cpioedit,SIGNAL(cellActivated(int,int)),SLOT(cpio_process_file(int,int)));
  connect(cpioedit,SIGNAL(cellDoubleClicked(int,int)),SLOT(cpio_process_file(int,int)));
  EditorLayout->addWidget(cpioedit);
  cpioedit->show();

}

//*********************************************************************
//* Уничтожение таблицы файлов
//*********************************************************************
void MainWindow::cpio_delete_list() {
 
disconnect(cpioedit,SIGNAL(cellActivated(int,int)),this,SLOT(cpio_process_file(int,int)));  
disconnect(cpioedit,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(cpio_process_file(int,int)));  
}

//*********************************************************************
//* Приемник сигнала выбора файла/каталога
//*********************************************************************
void MainWindow::cpio_process_file(int row, int col) {

QList<cpfiledir*>* subdir;
printf("\n col=%i row=%i current=%i",col,row,cpioedit->currentRow()); fflush(stdout);
if (row<0) return;
QString sname=cpioedit->item(row,0)->text();
// printf("\n subname = %s",sname.toLocal8Bit().data()); fflush(stdout);
subdir=find_dir((char*)sname.toLocal8Bit().data(), currentdir);
if (subdir == 0) return;
if (cpioedit != 0) {
  cpio_delete_list();
  EditorLayout->removeWidget(cpioedit);
  delete cpioedit;
  cpioedit=0;
  cpio_create_list(subdir);
}  
}

