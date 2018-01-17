// 
//  Копирование заголовков разделов
// 
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "headcopy.h"

//***************************************
//* Конструктор класса 
//***************************************
headcopy::headcopy(QWidget *parent) : QDialog(parent) {

uint32_t i;  
char str[100];

setupUi(this);
setWindowFlags (windowFlags() & ~Qt::WindowContextHelpButtonHint); 

// формируем список источников копирования
for(i=0;i<ptable->index();i++) {
  sprintf(str,"%02i %s",i,ptable->name(i));
  from->insertItem(i,str);
}
// формируем список приемников копирования
to->insertItem(0,"все разделы");
for(i=0;i<ptable->index();i++) {
  sprintf(str,"%02i %s",i,ptable->name(i));
  to->insertItem(i+1,str);
}
from->setCurrentIndex(0); 
to->setCurrentIndex(0); 
}
  
//*****************************************
//*  Копирование заголовка раздела
//*****************************************
int headcopy::exec() {

int src,dst,i;

src=from->currentIndex();
dst=to->currentIndex()-1;
for(i=0;i<ptable->index();i++) {
  if ((i == dst) || (dst == -1)) {
    memcpy(ptable->hptr(i),ptable->hptr(src),sizeof(struct pheader));
  }
}  
  
accept();
return 0;
}

//-------------------- интерфейс для функции ------------
void head_copy() {

headcopy* ul=new headcopy;
ul->show();
}  
