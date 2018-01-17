#include <QtWidgets>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parts.h"

// сигнатура заголовка таблицы  
const uint8_t headmagic[16]={0x70, 0x54, 0x61, 0x62, 0x6c, 0x65, 0x48, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};  
  

//*********************************************************
//* Проверка раздела - является ли он таблицей разделов?
//*********************************************************
int is_ptable(void* ptimage) {

struct ptable_t* pt=(ptable_t*)ptimage;
if(memcmp(pt->head,headmagic,16) == 0) return 1;
else return 0;
}


//*********************************************
//* Формирование таблицы mtd-разделов
//*********************************************
void parts_fill(QTableWidget* ptedit,void* ptimage) {

int pnum;
struct ptable_t* pt=(ptable_t*)ptimage;
QTableWidgetItem* item;
char str[100];

for(pnum=0;
   (pt->part[pnum].name[0] != 0) &&
   (strcmp(pt->part[pnum].name,"T") != 0);
   pnum++) {
   
   // добавлякм строку
   ptedit->setRowCount(pnum+1);
   item=new QTableWidgetItem(pt->part[pnum].name);
   item->setFlags(Qt::ItemIsEditable);
   item->setForeground(QBrush(Qt::red));
   ptedit->setItem(pnum,0,item);

   sprintf(str,"%i",pt->part[pnum].start/0x20000);
   item=new QTableWidgetItem(str);
   ptedit->setItem(pnum,1,item);

   sprintf(str,"%i",pt->part[pnum].length/0x20000);
   item=new QTableWidgetItem(str);
   ptedit->setItem(pnum,2,item);

   sprintf(str,"%08x",pt->part[pnum].lsize);
   item=new QTableWidgetItem(str);
   if (pt->part[pnum].lsize != 0) item->setForeground(QBrush(Qt::green));
   ptedit->setItem(pnum,3,item);

   sprintf(str,"%08x",pt->part[pnum].loadaddr);
   item=new QTableWidgetItem(str);
   ptedit->setItem(pnum,4,item);
   if (pt->part[pnum].loadaddr != 0) item->setForeground(QBrush(Qt::green));

   sprintf(str,"%08x",pt->part[pnum].entry);
   item=new QTableWidgetItem(str);
   if (pt->part[pnum].entry != 0) item->setForeground(QBrush(Qt::blue));
   ptedit->setItem(pnum,5,item);

   sprintf(str,"%08x",pt->part[pnum].nproperty);
   item=new QTableWidgetItem(str);
   ptedit->setItem(pnum,6,item);

   sprintf(str,"%08x",pt->part[pnum].type);
   item=new QTableWidgetItem(str);
   ptedit->setItem(pnum,7,item);

   sprintf(str,"%08x",pt->part[pnum].count);
   item=new QTableWidgetItem(str);
   ptedit->setItem(pnum,8,item);
}
ptedit->resizeColumnsToContents();

}

