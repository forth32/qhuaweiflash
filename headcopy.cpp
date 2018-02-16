// 
//  Копирование заголовков разделов
// 
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "ptable.h"

//**************************************************
//* Копирование заголовка раздела в другой раздел
//**************************************************
void head_copy() {

uint32_t i,res;  
char str[100];
int src,dst;

QDialog* qd=new QDialog;  
QGridLayout* lm=new QGridLayout(qd);

QFont font;
font.setPointSize(14);
font.setBold(true);
font.setWeight(75);

QLabel* label1 = new QLabel("Источник",qd);
label1->setFont(font);
lm->addWidget(label1,0,0);

QLabel* label2 = new QLabel("Приемник",qd);
label2->setFont(font);
lm->addWidget(label2,0,1);

QComboBox* from=new QComboBox(qd);
lm->addWidget(from,1,0);

QComboBox* to=new QComboBox(qd);
lm->addWidget(to,1,1);

QDialogButtonBox* buttonBox = new QDialogButtonBox(qd);
buttonBox->setOrientation(Qt::Horizontal);
buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
lm->addWidget(buttonBox,2,1);

QObject::connect(buttonBox, SIGNAL(accepted()), qd, SLOT(accept()));
QObject::connect(buttonBox, SIGNAL(rejected()), qd, SLOT(reject()));

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

res=qd->exec();

src=from->currentIndex();
dst=to->currentIndex()-1;

delete label1;
delete label2;
delete from;
delete to;
delete lm;
delete qd;
if (res !=  QDialog::Accepted) return;

// оператор подтвердил выполнение

for(i=0;i<ptable->index();i++) {
  if ((i == dst) || (dst == -1)) {
    memcpy(ptable->hptr(i),ptable->hptr(src),sizeof(struct pheader));
  }
}
}  
