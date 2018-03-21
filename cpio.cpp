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

//*********************************************************************
//* Конструктор класса редактора cpio
//*********************************************************************
cpioedit::cpioedit (int xpnum, QWidget* parent) : QWidget(parent) {
  
pnum=xpnum;
// образ раздела
pdata=ptable->iptr(pnum);
plen=ptable->psize(pnum);

// компоновщик окна
vlm=new QVBoxLayout(this);

// тулбар
toolbar=new QToolBar("Файловые операции",this);
vlm->addWidget(toolbar);

// загружаем весь cpio в списки
rootdir=load_cpio(pdata,plen);
// выводим корневой каталог
cpio_show_dir(rootdir,0);

// Пункты меню редактора
menu_extract=mw->menu_edit->addAction("Извлечь файл",this,SLOT(extract_file()),QKeySequence("F11"));

// открываем доступ к меню
mw->menu_edit->setEnabled(true);

}

//*********************************************************************
//* Деструктор класса cpio
//*********************************************************************
cpioedit::~cpioedit () {
  
delete rootdir;
// уничтожаем меню
mw->menu_edit->clear();
mw->menu_edit->setEnabled(false);

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

int i,j;
time_t ctime;
char tstr[100];
uint32_t fm;
char modestr[10];
int showsize;

cpiotable=new QTableWidget(0,6,this);

plst << "Name" << "size" << "Date" << "Mode" << "GID" << "UID"; //
cpiotable->setHorizontalHeaderLabels(plst);

currentdir=dir;

for (i=0;i<dir->count();i++) {
  cpiotable->setRowCount(cpiotable->rowCount()+1);
  
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

  cpiotable->setItem(i,0,item);
  if (i == 0) continue;

  // размер файла
  if (showsize) {
   str.sprintf("%i",dir->at(i)->fsize());
   item=new QTableWidgetItem(str);
   item->setFlags(Qt::ItemIsEditable);
   item->setForeground(QBrush(Qt::blue));
   cpiotable->setItem(i,1,item);
  } 
  
  // дата-время
  ctime=dir->at(i)->ftime();
  strftime(tstr,100,"%d-%b-%y  %H:%M",localtime(&ctime));
  str=tstr;
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::black));
  cpiotable->setItem(i,2,item);
  
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
  cpiotable->setItem(i,3,item);
  
  // gid
  str.sprintf("%i",dir->at(i)->fgid());
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::black));
  cpiotable->setItem(i,4,item);
  
  // uid
  str.sprintf("%i",dir->at(i)->fuid());
  item=new QTableWidgetItem(str);
  item->setFlags(Qt::ItemIsEditable);
  item->setForeground(QBrush(Qt::black));
  cpiotable->setItem(i,5,item);
} 
  cpiotable->resizeColumnsToContents();
  cpiotable->setShowGrid(false);
  cpiotable->setColumnWidth(0, 210);
  cpiotable->setColumnWidth(1, 100);

  cpiotable->sortByColumn(0,Qt::AscendingOrder);
  
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
void cpioedit::cpio_delete_list() {

vlm->removeWidget(cpiotable);
  
disconnect(cpiotable,SIGNAL(cellActivated(int,int)),this,SLOT(cpio_process_file(int,int)));  
disconnect(cpiotable,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(cpio_process_file(int,int)));  
delete cpiotable;
cpiotable=0;
}


//*********************************************************************
//* извлечение файла
//*********************************************************************
void cpioedit::extract_file() {

FILE* out;  
cpfiledir* fd;
QTableWidgetItem* item;
QString qfn;
int idx;

int row=cpiotable->currentRow();
item=cpiotable->item(row,0);
qfn=item->text();
idx=find_file(qfn,currentdir);
if (idx == -1) {
  // такой ошибки быть не должно - файл всегда должен быть найден
  return;
}  

fd=currentdir->at(idx);

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
//* Приемник сигнала выбора файла/каталога
//*********************************************************************
void cpioedit::cpio_process_file(int row, int col) {

QList<cpfiledir*>* subdir;
// printf("\n col=%i row=%i current=%i",col,row,cpiotable->currentRow()); fflush(stdout);
if (row<0) return;
QString sname=cpiotable->item(row,0)->text();
// printf("\n subname = %s",sname.toLocal8Bit().data()); fflush(stdout);
if (row != 0) subdir=find_dir((char*)sname.toLocal8Bit().data(), currentdir);
else subdir=currentdir->at(0)->subdir;
if (subdir == 0) return;
if (cpiotable != 0) {
  cpio_delete_list();
  cpio_show_dir(subdir,1);
}  
}

