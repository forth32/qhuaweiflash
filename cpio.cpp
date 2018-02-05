// 
//  Сохранение файла прошивки на диск
// 
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "MainWindow.h"
#include "cpio.h"


//*******************************************************
//* Загрузка в память файловой структуры cpio-архива
//*******************************************************
QVector<cpfiledir>* load_cpio(char* pimage, int len) {


// вектор корневого каталога 
QVector<cpfiledir>* root=new QVector<cpfiledir>*;

char* iptr=pimage;  // указатель на текущую позицию в образе раздела



