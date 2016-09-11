#include <QtWidgets>
#include <MainWindow.h>

#include <stdio.h>
#include <stdint.h>

#include "sio.h"
#include "ptable.h"
#include "flasher.h"
#include "usbloader.h"

#include "hexeditor/qhexedit.h"

// ссылка на селектор портов
QComboBox* pselector;

// Таблица разделов
ptable_list* ptable;
int npart=0;

// Методы класса главного окна
//=============================================
//*****************************************
//* Конструктор класса
//*****************************************
MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent) {
    
// Настройка элементов окна
setupUi(this);

RefreshPorts->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

pselector=PortSelector;
// создание класса таблицы разделов
ptable=new ptable_list;

// заполнение списка портов
find_ports();

// создание окна hex-редактора
hexedit=new QHexEdit(centralwidget);
hexedit->setObjectName(QStringLiteral("HexEditor"));
hexedit->setGeometry(QRect(230, 100, 600, 470));
hexedit->setAddressWidth(8);
hexedit->setOverwriteMode(true);
hexedit->hide();

}

//*****************************************
//* Десруктор класса
//*****************************************
MainWindow::~MainWindow()  {

delete ptable;  
}
  
//*************************************************
//  Поиск ttyUSB портов и сбор их имен в таблицу
//*************************************************
void MainWindow::find_ports() {

QDir fdir("/dev");

PortSelector->clear();
PortSelector->addItems(fdir.entryList((QStringList)"ttyUSB*",QDir::System,QDir::Name));
PortSelector->setCurrentIndex(0);
}

  
//*****************************************
//*  Добавление разделов из файла прошивки
//*****************************************
void MainWindow::AppendFwFile() {
  
QString fwname;
FILE* in;

QFileDialog* qf=new QFileDialog(this);
fwname=qf->getOpenFileName(0,"Выбор файла прошивки","","*.exe *.bin *.fw");
delete qf;
if (fwname.isEmpty()) return;
in=fopen(fwname.toLocal8Bit(),"r");
if (in == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка открытия файла");
  return;
}  

// Поиск разделов и формирование таблицы разделов

ptable->findparts(in); 
regenerate_partlist();
partlist->setCurrentRow(0);
SelectPart();  
}


//********************************************
//*  Формирование экранного списка разделов
//********************************************
void MainWindow::regenerate_partlist() {

int i;
QString str;
partlist->clear();
for (i=0;i<ptable->index();i++) {
  str.sprintf("%02x - %s",ptable->code(i)>>16,ptable->name(i));
  partlist->addItem(str);
}  
}  

//*****************************************
//*  Выбор нового файла прошивки
//*****************************************
void MainWindow::SelectFwFile() {
  
menu_part->setEnabled(0);
Menu_Oper_flash->setEnabled(0);
fileappend->setEnabled(0);
filesave->setEnabled(0);
ptable->clear();
AppendFwFile();
if (ptable->index() != 0) { 
  menu_part->setEnabled(1);
  Menu_Oper_flash->setEnabled(1);
  fileappend->setEnabled(1);
  filesave->setEnabled(1);
}
}

//*****************************************
//*  Запись полного файла прошивки
//*****************************************
void MainWindow::SaveFwFile() {

QString filename="firmware.fw";
FILE* out;
int i;

QFileDialog::getSaveFileName(this,"Имя файла",filename,"firmware (*.fw);;All files (*.*)");
if (filename.isEmpty()) return;
out=fopen(filename.toLocal8Bit(),"w");
if (out == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка создания файла");
  return;
}
for(i=0;i<ptable->index();i++) ptable->save_part(i,out);
fclose(out);
}



//*****************************************
//*  Выбор раздела из списка
//*****************************************
void MainWindow::SelectPart() {

QString txt;  

int idx=partlist->currentRow();

// Вывод значений заголовка
txt.sprintf("%-8.8s",ptable->platform(idx));
Platform_input->setText(txt);

txt.sprintf("%-16.16s",ptable->date(idx));
Date_input->setText(txt);

txt.sprintf("%-16.16s",ptable->time(idx));
Time_input->setText(txt);

txt.sprintf("%-32.32s",ptable->version(idx));
Version_input->setText(txt);

// формирование окна hex-редактора
hexcup.setRawData((char*)ptable->iptr(idx),ptable->psize(idx));
hexedit->setData(hexcup);
hexedit->setCursorPosition(0);
hexedit->show();
  
}  


//*****************************************
//*  Сохранение раздела на диск
//*****************************************
void MainWindow::Menu_Part_Store() {
  
int np=partlist->currentRow();
QString filename;
QString str;
FILE* out;

filename.sprintf("%02i-%08x-%s.fw",np,ptable->code(np),ptable->name(np));
QFileDialog::getSaveFileName(this,"Имя файла",filename,"firmware (*.fw);;All files (*.*)");
if (filename.isEmpty()) return;
out=fopen(filename.toLocal8Bit(),"w");
if (out == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка создания файла");
  return;
}
ptable->save_part(np,out);
fclose(out);
}

//*****************************************
//*  Извлечение образа раздела на диск
//*****************************************
void MainWindow::Menu_Part_Extract() {
  
int np=partlist->currentRow();
QString filename;
QString str;
FILE* out;

filename.sprintf("%02i-%08x-%s.bin",np,ptable->code(np),ptable->name(np));
QFileDialog::getSaveFileName(this,"Имя извлекаемого файла",filename,"image (*.bin);;All files (*.*)");
if (filename.isEmpty()) return;
out=fopen(filename.toLocal8Bit(),"w");
if (out == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка открытия файла");
  return;
}
fwrite(ptable->iptr(np),1,ptable->psize(np),out);
fclose(out);
}


//*****************************************
//*  Замена образа раздела 
//*****************************************
void MainWindow::Menu_Part_Replace() {

int np=partlist->currentRow();
QString filename;
QString str;
FILE* in;

filename.sprintf("%02i-%08x-%s.bin",np,ptable->code(np),ptable->name(np));
QFileDialog::getOpenFileName(this,"Имя файла с образом раздела",filename,"image (*.bin);;All files (*.*)");
if (filename.isEmpty()) return;
in=fopen(filename.toLocal8Bit(),"r");
if (in == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка открытия файла");
  return;
}
ptable->loadimage(np,in);
}

//*****************************************
//*  Удаление раздела 
//*****************************************
void MainWindow::Menu_Part_Delete() {

int32_t ci=partlist->currentRow(); 

if (ptable->index() == 1) return; // последний раздел удалять нельзя
ptable->delpart(ci);
regenerate_partlist();
if (ci< (ptable->index()-1)) partlist->setCurrentRow(ci);
else partlist->setCurrentRow(ptable->index()-1);
  
}


//*****************************************
//*  Перемещение раздела вверх 
//*****************************************
void MainWindow::Menu_Part_MoveUp() {
  
int32_t ci=partlist->currentRow(); 
  
ptable->moveup(ci);
regenerate_partlist();
if (ci>0) partlist->setCurrentRow(ci-1);
else partlist->setCurrentRow(0);
}

//*****************************************
//*  Перемещение раздела вниз
//*****************************************
void MainWindow::Menu_Part_MoveDown() {

int32_t ci=partlist->currentRow(); 
  
ptable->movedown(ci);
regenerate_partlist();
if (ci<(ptable->index()-1)) partlist->setCurrentRow(ci+1);
else partlist->setCurrentRow(ptable->index()-1);
}


//********************************************
// Разрешение редактирования полей заголовка
//********************************************
void MainWindow::Menu_Part_EditHeader() {
  
  
Platform_input->setReadOnly(0);
Date_input->setReadOnly(0);
Time_input->setReadOnly(0);
Version_input->setReadOnly(0);
}

//********************************************
//* Запись областей редактирования заголовка
//********************************************
void MainWindow::HeaderChanged() {

int32_t ci=partlist->currentRow(); 

if (Platform_input->isModified())  strncpy((char*)ptable->hptr(ci)->unlock,Platform_input->text().toLocal8Bit(),8);
if (Date_input->isModified())  strncpy((char*)ptable->hptr(ci)->date,Date_input->text().toLocal8Bit(),16);
if (Time_input->isModified())  strncpy((char*)ptable->hptr(ci)->time,Time_input->text().toLocal8Bit(),16);
if (Version_input->isModified())  strncpy((char*)ptable->hptr(ci)->version,Version_input->text().toLocal8Bit(),32);
}

//********************************************
// Запрещение редактирования полей заголовка
//********************************************
void MainWindow::Disable_EditHeader() {
  
  
Platform_input->setReadOnly(0);
Date_input->setReadOnly(0);
Time_input->setReadOnly(0);
Version_input->setReadOnly(0);
}

//********************************************
// Запуск диалога прошивальщика
//********************************************
void MainWindow::Start_Flasher() {

if (PortSelector->count() == 0) {
   QMessageBox::critical(0,"Ошибка","Не найдены последовательне порты");
   return;
}
  
flasher* fl=new flasher;
fl->show();
}

//********************************************
//  Перезагрузка модема
//********************************************
void MainWindow::Reboot_modem() {

if (PortSelector->count() == 0) {
   QMessageBox::critical(0,"Ошибка","Не найдены последовательне порты");
   return;
}
open_port();
modem_reboot();  
close_port();
QMessageBox::information(0,"ОK","Команда перезагрузки передана в модем");
}


//********************************************
// Запуск udb-загрузчика
//********************************************
void MainWindow::usbdload() {

if (PortSelector->count() == 0) {
   QMessageBox::critical(0,"Ошибка","Не найдены последовательне порты");
   return;
}
  
usbloader* ul=new usbloader;
ul->show();
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@222

int main(int argc, char* argv[]) {

QApplication app(argc, argv);
   
MainWindow* mw = new  MainWindow(0);
mw->show();
return app.exec();
}
