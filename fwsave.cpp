// 
//  Сохранение файла прошивки на диск
// 
#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "fwsave.h"
#include "ptable.h"
#include "signver.h"

extern QString* fwfilename;
QLineEdit* filename;


//***************************************
//* Выбор имени файла
//***************************************
void fsdialog::browse() {

QString fn=*fwfilename;

fn=QFileDialog::getSaveFileName(this,"Имя файла",fn,"firmware (*.fw);;All files (*.*)");
if (fn.isEmpty()) return;
  
filename->setText(fn);
}
  
//***************************************
//*  Слот exec()
//***************************************
int fsdialog::exec() {

// return QDialog::Accepted;
 return QDialog::exec(); 
}
  
//****************************************************************
//* Процедура сохранения образа прошивки в новом файле
//****************************************************************
void fw_saver() {

uint32_t i,res;  
FILE* out;
uint8_t hdr[92];
uint8_t zflag=0;
uint32_t percent;
char fname[200];
int dlcode;

QString deffilename=*fwfilename;
  
QDialog* fsd=new fsdialog; 
QGridLayout* lm = new QGridLayout(fsd);

QLabel* label_3 = new QLabel("Имя файла:",fsd);
lm->addWidget(label_3, 0, 0, 1, 1);

filename = new QLineEdit(fsd);
filename->setText(deffilename);
// filename->setReadOnly(true);
lm->addWidget(filename, 0, 1, 1, 2);

QToolButton* fselector = new QToolButton(fsd);
fselector->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon))); 
// fselector->setText("...");
lm->addWidget(fselector, 0, 3, 1, 1);

QCheckBox* compressflag = new QCheckBox("Сжать образы разделов",fsd);
lm->addWidget(compressflag, 2, 0, 1, 3);

QDialogButtonBox* buttonBox = new QDialogButtonBox(fsd);
buttonBox->setOrientation(Qt::Horizontal);
buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
lm->addWidget(buttonBox, 3, 0, 1, 3);

QComboBox* fcode = new QComboBox(fsd);
lm->addWidget(fcode, 1, 1, 1, 3);
// формируем список типов прошивок
for(i=0;i<8;i++) {
 fcode->insertItem(i,fw_description(i));
}   
fcode->setCurrentIndex(dload_id&7); 

QLabel* label = new QLabel("Код файла прошивки:",fsd);
lm->addWidget(label, 1, 0, 1, 1);


QObject::connect(buttonBox, SIGNAL(accepted()), fsd, SLOT(accept()));
QObject::connect(buttonBox, SIGNAL(rejected()), fsd, SLOT(reject()));
QObject::connect(fselector, SIGNAL(clicked()), fsd, SLOT(browse()));

res=fsd->exec();

if (compressflag->isChecked()) zflag=1;
strcpy(fname,filename->displayText().toLocal8Bit());
dlcode=fcode->currentIndex();
delete fsd;

if (res != QDialog::Accepted) return;
     
out=fopen(fname,"w");
if (out == 0) {
    QMessageBox::critical(0,"Ошибка","Ошибка создания файла");
    return;
}

// записываем заголовок - upgrade state
bzero(hdr,sizeof(hdr));
// выделяем код типа прошивки
hdr[0]=dlcode;
if (signlen != -1) hdr[0]|=0x8;
fwrite(hdr,1,sizeof(hdr),out);

// Формируем окно прогресс-бара
QWidget* pb=new QWidget();
QVBoxLayout* plm=new QVBoxLayout(pb);

QLabel* lb = new QLabel("Сохранение разделов",pb);
QFont font;
font.setPointSize(14);
font.setBold(true);
font.setWeight(75);
lb->setFont(font);
plm->addWidget(lb);

QProgressBar* fbar = new QProgressBar(pb);
fbar->setValue(0);
plm->addWidget(fbar);

pb->show();

// записываем образы всех разделов

for(i=0;i<ptable->index();i++) {
  ptable->save_part(i,out,zflag);
  percent=(i+1)*100/(ptable->index());
  fbar->setValue(percent);
  QCoreApplication::processEvents();
}
delete pb;  

fclose(out);
}  
    
