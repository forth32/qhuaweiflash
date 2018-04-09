// 
//  Редактор cpio-разделов
// 
#include <QtCore/QVariant>
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "MainWindow.h"
#include "cpio.h"
#include "viewer.h"
#include "hexfileviewer.h"

//*********************************************************************
//* Конструктор класса редактора cpio
//*********************************************************************
cpioedit::cpioedit (int xpnum,QMenuBar* mbar, QWidget* parent) : QWidget(parent) {

int res;
char filename[512];

menubar=mbar;  
pnum=xpnum;
// образ раздела
pdata=ptable->iptr(pnum);
plen=ptable->psize(pnum);

// компоновщик окна
vlm=new QVBoxLayout(this);

// тулбар
toolbar=new QToolBar("Файловые операции",this);
vlm->addWidget(toolbar);

// меню редактора
menu_edit = new QMenu("CPIO-Редактор",menubar);
menubar->addAction(menu_edit->menuAction());

// Пункты меню редактора
menu_edit->addAction(QIcon::fromTheme("document-save"),"Извлечь файл",this,SLOT(extract_file()),QKeySequence("F11"));
menu_edit->addAction(QIcon::fromTheme("object-flip-vertical"),"Заменить файл",this,SLOT(replace_file()),0);
menu_edit->addAction(QIcon::fromTheme("edit-delete"),"Удалить файл",this,SLOT(delete_file()),QKeySequence("Del"));
menu_edit->addAction(QIcon::fromTheme("list-add"),"HEX-просмотр/редактор",this,SLOT(hexedit_file()),QKeySequence("F2"));
menu_edit->addAction(QIcon::fromTheme("list-add"),"Текстовый просмотр",this,SLOT(view_file()),QKeySequence("F3"));
menu_edit->addAction(QIcon::fromTheme("list-add"),"Текстовый редактор",this,SLOT(edit_file()),QKeySequence("F4"));

menu_edit->addAction(QIcon::fromTheme("file-save"),"Сохранить изменения",this,SLOT(saveall()),QKeySequence("Ctrl+S"));

// Пункты тулбара
toolbar->addAction(QIcon::fromTheme("document-save"),"Извлечь файл",this,SLOT(extract_file()));
toolbar->addAction(QIcon::fromTheme("object-flip-vertical"),"Заменить файл",this,SLOT(replace_file()));
toolbar->addAction(QIcon::fromTheme("edit-delete"),"Удалить файл",this,SLOT(delete_file()));
toolbar->addAction(QIcon::fromTheme("list-add"),"Текстовый просмотр",this,SLOT(view_file()));
toolbar->addAction(QIcon::fromTheme("list-add"),"Текстовый редактор",this,SLOT(edit_file()));

// открываем доступ к меню
menu_edit->setEnabled(true);

// загружаем весь cpio в списки
uint8_t* iptr=pdata;  // указатель на текущую позицию в образе раздела
rootdir=new QList<cpfiledir*>;
// Цикл разбора cpio-потока
while(iptr < (pdata+plen)) {
  // Ищем сигнатуру заголовка очередного файла
  while(1) {
   if (iptr >= (pdata+plen)) {
     QMessageBox::critical(0,"Ошибка CPIO","Не обнаружен ограничитель потока TRAILER!!!");
     goto ldone;
   }  
   if (is_cpio(iptr)) break; // нашли сигнатуру
   iptr++; // ищем ее дальше
  }  
 extract_filename(iptr,filename);
 if (strncmp(filename,"TRAILER!!!",10) == 0) break;
 res=cpio_load_file(iptr,rootdir,plen,filename);
 if (res == 0) break;
 iptr+=res;
}
ldone:
// выводим корневой каталог
cpio_show_dir(rootdir,0);

}

//*********************************************************************
//* Деструктор класса cpio
//*********************************************************************
cpioedit::~cpioedit () {

QMessageBox::StandardButton reply;
cpfiledir* fd; 

// Проверяем, не изменлось ли что-нибудь внутри  
if (is_modified) {
  reply=QMessageBox::warning(this,"Запись раздела","Содержимое раздела изменено, сохранить?",QMessageBox::Ok | QMessageBox::Cancel);
  if (reply == QMessageBox::Ok) repack_cpio();
}  
// удаляем элементы корневого каталога
qDeleteAll(*rootdir);
// очищаем корневой каталог
rootdir->clear();  
// удаляем корневой каталог
delete rootdir;

// уничтожаем меню
delete menu_edit;

}



//*********************************************************************
//* Сохранение изменений
//*********************************************************************
void cpioedit::saveall() {
  
repack_cpio();
is_modified=false;
}

//*************************************************************
//*  Формирование списка файлов
//*
//* focusmode - разрешает установку фокуса на окно просмотра
//*************************************************************
void cpioedit::cpio_show_dir(QList<cpfiledir*>* dir, int focusmode) {

QTableWidgetItem* item;
QString str;
QStringList(plst);
QStringList(hlist);

int i,j;
time_t ctime;
char tstr[100];
uint32_t fm;
char modestr[10];
int showsize;

cpiotable=new QTableWidget(0,7,this);

plst <<"idx" << "Name" << "size" << "Date" << "Mode" << "GID" << "UID"; 
cpiotable->setHorizontalHeaderLabels(plst);

currentdir=dir;

cpiotable->setRowCount(dir->count()); //cpiotable->rowCount()+1);
for (i=0;i<dir->count();i++) {
  hlist <<""; 
  // индекс файла в векторе
  str.sprintf("%i",i);
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::black));
  cpiotable->setItem(i,0,item);
  
  // имя файла
  str=dir->at(i)->cfname();
  item=new QTableWidgetItem(str);
  // Выбор иконки файла
  showsize=0;
  if (i == 0) item->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowBack))); 
  else if (dir->at(i)->subdir != 0) item->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon))); 
  else if (((dir->at(i)->fmode())&C_ISLNK) == C_ISLNK) {
    // симлмнк
    item->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_FileLinkIcon)));
    // добавляем к имени симлинка ссылку на имя файла
    str.append(" -> ");
    str.append(dir->at(i)->fdata()); 
    item->setText(str);
  }  
  else  {
    // выполняемые файлы
    if ((((dir->at(i)->fmode())&C_IXUSR) != 0)) item->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon)));
    // невыполняемые файлы
    else item->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon)));
    // разрешить показ размера
    showsize=1;
  }  

  cpiotable->setItem(i,1,item);
  if (i == 0) continue;

  // размер файла
  if (showsize) {
   str.sprintf("%i",dir->at(i)->fsize());
   item=new QTableWidgetItem(str);
   item->setFlags(Qt::ItemIsEditable);
   item->setForeground(QBrush(Qt::blue));
   cpiotable->setItem(i,2,item);
  } 
  
  // дата-время
  ctime=dir->at(i)->ftime();
  strftime(tstr,100,"%d-%b-%y  %H:%M",localtime(&ctime));
  str=tstr;
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::black));
  cpiotable->setItem(i,3,item);
  
  // атрибуты доступа
  fm=dir->at(i)->fmode();
  strcpy(modestr,"rwxrwxrwx");
  for (j=0;j<9;j++) {
    if (((fm>>j)&1) == 0) modestr[8-j]='-';
  }  
  str=modestr;
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::red));
  cpiotable->setItem(i,4,item);
  
  // gid
  str.sprintf("%i",dir->at(i)->fgid());
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::black));
  cpiotable->setItem(i,5,item);
  
  // uid
  str.sprintf("%i",dir->at(i)->fuid());
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::black));
  cpiotable->setItem(i,6,item);

} 
  //------------------------------------
  // прячем индексы файлов
  cpiotable->setColumnHidden(0,true);
  
  // прячем вертикальные заголовки
  cpiotable->setVerticalHeaderLabels(hlist);

  
  cpiotable->resizeColumnsToContents();
  cpiotable->setShowGrid(false);
  cpiotable->setColumnWidth(1, 250);
  cpiotable->setColumnWidth(2, 100);

  cpiotable->sortByColumn(1,Qt::AscendingOrder);
  
  connect(cpiotable,SIGNAL(cellActivated(int,int)),SLOT(cpio_process_file(int,int)));
  connect(cpiotable,SIGNAL(cellDoubleClicked(int,int)),SLOT(cpio_process_file(int,int)));
  vlm->addWidget(cpiotable);
  cpiotable->show();
  if (focusmode) cpiotable->setFocus();
  cpiotable->setCurrentCell(0,0);
  
}

//*********************************************************************
//* Уничтожение таблицы файлов
//*********************************************************************
void cpioedit::cpio_hide_dir() {

vlm->removeWidget(cpiotable);
  
disconnect(cpiotable,SIGNAL(cellActivated(int,int)),this,SLOT(cpio_process_file(int,int)));  
disconnect(cpiotable,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(cpio_process_file(int,int)));  
delete cpiotable;
cpiotable=0;
}

//*********************************************************************
//* Получение индекса текущего файла в векторе каталога
//*********************************************************************
int cpioedit::current_file_index() {

QTableWidgetItem* item;
QString qfn;
int idx;
int row=cpiotable->currentRow();
item=cpiotable->item(row,0);
qfn=item->text();
idx=qfn.toUInt();
return idx;
}

//*********************************************************************
//* Получение ссылки на описатель текущего файла
//*********************************************************************
cpfiledir* cpioedit::selected_file() {

return currentdir->at(current_file_index());
}

//*********************************************************************
//* Удаление файла
//*********************************************************************
void cpioedit::delete_file() {
  
int idx;
int row=cpiotable->currentRow();

idx=current_file_index(); // позиция файла в векторе
delete selected_file();   // удаляем описатель файла
currentdir->removeAt(idx); // сносим файл из списка
// перерисовываем таблицу
cpio_hide_dir();
cpio_show_dir(currentdir,true);
cpiotable->setCurrentCell(row,0);
}


//*********************************************************************
//* извлечение файла
//*********************************************************************
void cpioedit::extract_file() {

FILE* out;  
cpfiledir* fd;

fd=selected_file();

if (((fd->fmode()) & C_ISREG) == 0) {
  // нерегулярный файл - его извлекать нельзя
  QMessageBox::critical(0,"Ошибка","Нерегулярные файлы извлекать нельзя");  
  return;
}

QString fn=fd->cfname();

fn=QFileDialog::getSaveFileName(this,"Сохранение файла",fn,"All files (*.*)");
if (fn.isEmpty()) return;
out=fopen(fn.toLocal8Bit().data(),"w");
fwrite(fd->fdata(),1,fd->fsize(),out);
fclose(out);
}

//*********************************************************************
//* замена файла
//*********************************************************************
void cpioedit::replace_file() {

cpfiledir* fd;
QString fn;
uint32_t fsize;

fd=selected_file();

if (((fd->fmode()) & C_ISREG) == 0) {
  // нерегулярный файл - его извлекать нельзя
  QMessageBox::critical(0,"Ошибка","Нерегулярные файлы заменять нельзя");  
  return;
}

fn=QFileDialog::getOpenFileName(this,"Замена файла",fn,"All files (*.*)");
if (fn.isEmpty()) return;

QFile in(fn,this);
if (!in.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(0,"Ошибка","Ошибка чтения файла");
    return;
}
fsize=in.size();
uint8_t* fbuf=new uint8_t[fsize]; // файловый буфер
in.read((char*)fbuf,fsize);
in.close();
fd->replace_data(fbuf,fsize);
delete [] fbuf;
}

//*********************************************************************
//* Вызов файлового редактора
//*********************************************************************
void cpioedit::fileeditor(bool readonly) {

cpfiledir* fd;

fd=selected_file();

if (((fd->fmode()) & C_ISREG) == 0) {
  // нерегулярный файл - его извлекать нельзя
  QMessageBox::critical(0,"Ошибка","Нерегулярные файлы просматривать/редактировать нельзя");  
  return;
}

view=new viewer(fd,readonly);  
// сигнал модификации
connect(view,SIGNAL(changed()),this,SLOT(setModified()));

}  

//*********************************************************************
//* текстовый редактор
//*********************************************************************
void cpioedit::view_file() { fileeditor(true); }
void cpioedit::edit_file() { fileeditor(false); }

//*********************************************************************
//* Вызов hex-редактора
//*********************************************************************
void cpioedit::hexedit_file() {

cpfiledir* fd;

fd=selected_file();

if (((fd->fmode()) & C_ISREG) == 0) {
  // нерегулярный файл - его извлекать нельзя
  QMessageBox::critical(0,"Ошибка","Нерегулярные файлы просматривать/редактировать нельзя");  
  return;
}

hview=new hexfileviewer(fd);  

// сигнал модификации
connect(hview,SIGNAL(changed()),this,SLOT(setModified()));
}  



//*********************************************************************
//* Приемник сигнала выбора файла/каталога
//*********************************************************************
void cpioedit::cpio_process_file(int row, int col) {

QList<cpfiledir*>* subdir;

if (row<0) return;

if (row != 0) subdir=selected_file()->subdir;
else subdir=currentdir->at(0)->subdir;

if (subdir == 0) return; // не каталог
if (cpiotable != 0) { // не корневой каталог
  cpio_hide_dir();
  cpio_show_dir(subdir,1);
}  
}

//*********************************************************************
//* Перепаковка cpio-раздела обратно
//*********************************************************************
void cpioedit::repack_cpio() {
  
uint8_t* ndata=new uint8_t[fullsize(rootdir)+4096];
uint32_t nlen=0;
int i;

for(i=0;i<rootdir->count(); i++) {
  nlen+=rootdir->at(i)->store_cpio(ndata+nlen);
}

// хвост cpio-файла
bzero(ndata+nlen,128);
strcpy((char*)(ndata+nlen),"07070100000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000B00000000TRAILER!!!");
nlen+=128;

ptable->replace(pnum,ndata,nlen);
pdata=ndata;
plen=nlen;
}

  
