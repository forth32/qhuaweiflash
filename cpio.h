// 
//  Сохранение файла прошивки на диск
// 
#ifndef _CPIO_H
#define _CPIO_H

#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

// #include "MainWindow.h"

//************************************************************
//* Заголовок элемента архива
//************************************************************
struct cpio_header {
   char    c_magic[6];
   char    c_ino[8];
   char    c_mode[8];
   char    c_uid[8];
   char    c_gid[8];
   char    c_nlink[8];
   char    c_mtime[8];
   char    c_filesize[8];
   char    c_devmajor[8];
   char    c_devminor[8];
   char    c_rdevmajor[8];
   char    c_rdevminor[8];
   char    c_namesize[8];
   char    c_check[8];
};

//*****************************************************
//* Класс-хранилище элемента файловой системы
//*****************************************************
class cpfiledir {
  char* phdr; // ссылка на заголовок
  char* pname; // ссылка на имя файла
  char* pbody; // ссылка на тело файла
  QVector<cpfiledir>* subdir=0; // ссылка на контейнер поддиректории

  int isdir(){
    if (subdir == 0) return 0;
    else return 1;
  }
};

#endif