// 
//  Редактор cpio-файлов
// 
#ifndef _CPIO_H
#define _CPIO_H

#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "ptable.h"
#include "viewer.h"
#include "hexfileviewer.h"
#include "cpfiledir.h"

//*****************************************************
//* Класс редактора cpio-разделов
//*****************************************************
class cpioedit: public QWidget {
  
Q_OBJECT

int pnum;
// указатели на образ раздела
uint8_t* pdata;
uint32_t plen;

QToolBar* toolbar;
QTableWidget* cpiotable=0;
QVBoxLayout* vlm;

viewer* view; // окно просмотра файлов
hexfileviewer* hview; // окно hex-редактора

QList<cpfiledir*>* rootdir=0;   // указатель на вектор корневого раздела
QList<cpfiledir*>* currentdir;  // вектор текущего каталога
void cpio_hide_dir();
int current_file_index();
cpfiledir* selected_file();
void cpio_show_dir(QList<cpfiledir*>* dir, int focusmode);
void fileeditor(bool readonly);
void repack_cpio();

// флаг изменения раздела
bool is_modified=false;

QMenuBar* menubar;
QMenu* menu_edit;

public:
cpioedit(int xpnum,QMenuBar* mbar, QWidget* parent); 
~cpioedit();


public slots:
void cpio_process_file(int, int); // обработка выбора файла
void extract_file();  // извлекалка файлов
void replace_file();  // замена файлов
void delete_file();  // удаление файлов
void view_file();   // просмотр
void edit_file();   // просмотр
void hexedit_file();   // hex-просмотр/редактор
void setModified() {is_modified=true;}  // установка признака модификации содержимого архива
void saveall();
void menuenabler();
void go_up();
};


#endif