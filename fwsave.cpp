// 
//  Сохранение файла прошивки на диск
// 
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "fwsave.h"

extern QString* fwfilename;

//***************************************
//* Конструктор класса 
//***************************************
fwsave::fwsave(QWidget *parent) : QDialog(parent) {

QString deffilename=*fwfilename;
QString deffc="0D";

setupUi(this);
setWindowFlags (windowFlags() & ~Qt::WindowContextHelpButtonHint); 
filename->setText(deffilename);
fcode->setText(deffc); 
}

//***************************************
//* Выбор имени файла
//***************************************
void fwsave::browse() {

QString fn=*fwfilename;

fn=QFileDialog::getSaveFileName(this,"Имя файла",fn,"firmware (*.fw);;All files (*.*)");
if (fn.isEmpty()) return;
  
filename->setText(fn);
}
  
//*****************************************
//*  Запись на диск полного файла прошивки
//*****************************************
int fwsave::exec() {

FILE* out;
int i;
uint8_t hdr[92];
uint32_t filecode;
uint8_t zflag=0;
uint32_t percent;

out=fopen(filename->displayText().toLocal8Bit(),"w");
if (out == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка создания файла");
  reject();
  return -1;
}

// выделяем код типа прошивки
sscanf(fcode->displayText().toLocal8Bit(),"%x",&filecode);

// записываем заголовок - upgrade state
bzero(hdr,sizeof(hdr));
hdr[0]=filecode;
fwrite(hdr,1,sizeof(hdr),out);

// записываем образы всех разделов
pfindbar* pb=new pfindbar;
pb->show();

if (compressflag->isChecked()) zflag=1;

for(i=0;i<ptable->index();i++) {
  ptable->save_part(i,out,zflag);
  percent=(i+1)*100/(ptable->index());
  pb->fbar->setValue(percent);
  QCoreApplication::processEvents();
}
delete pb;
fclose(out);
accept();
return 0;
}

  
//-------------------- интерфейс для функции сохранения ------------
void fw_saver() {

fwsave* ul=new fwsave;
ul->show();
}  
    
