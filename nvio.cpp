//------------------- Библиотека для работы со структурой бинарный nv-файлов --------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "nvexplorer.h"

// Максимально допустимый размер ячеек
#define max_item_len 10000

//******************************************************
// Получение смещения до начала файла по номеру файла
//******************************************************
uint32_t nvexplorer::fileoff(int fid) {

int i;
for (i=0;i<nvhd.file_num;i++) {
  if (flist[i].id == fid) return flist[i].offset;
}
printf("\n - Ошибка структуры файла - компоненты #%i не существует\n",fid);
exit(1);
}

//******************************************************
// Получение индкса по номеру файла
//******************************************************
int32_t nvexplorer::fileidx(int fid) {

int i;

for (i=0;i<nvhd.file_num;i++) {
  if (flist[i].id == fid) return i;
}
return -1;
}


//******************************************************
// Получение смещения до начала ячейки по ее индексу
//******************************************************
uint32_t nvexplorer::itemoff_idx(int idx) {

return itemlist[idx].off+fileoff(itemlist[idx].file_id);
}


//******************************************************
//* Получение индекса ячейки по ее id
//*  возврат -1 - ячейка не найдена
//******************************************************
int32_t nvexplorer::itemidx(int item) {
  
int i;

for(i=0;i<nvhd.item_count;i++) {
  if (itemlist[i].id == item) return i;
}
return -1;
}

//******************************************************
// Получение смещения до начала ячейки по ее номеру
//******************************************************
int32_t nvexplorer::itemoff (int item) {

int idx=itemidx(item);
if (item == -1) return -1;
return itemoff_idx(idx);
}

//******************************************************
// Получение размера ячейки по ее номеру
//******************************************************
int32_t nvexplorer::itemlen (int item) {

int idx=itemidx(item);
if (idx == -1) return -1;
return itemlist[idx].len;
}

//**********************************************
//* Загрузка ячейки в буфер
//**********************************************
int nvexplorer::load_item(int item, char* buf) {
  
int idx;

idx=itemidx(item);
if (idx == -1) return -1; // не найдена
memcpy(buf,pdata+itemoff_idx(idx),itemlist[idx].len);
return itemlist[idx].len;
}

