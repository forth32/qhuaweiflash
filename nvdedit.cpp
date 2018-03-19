// редактор раздела nvdload
#include "nvdedit.h"
#include "MainWindow.h"
#include <string.h>
#include "ptable.h"

//********************************************************************
//* Конструктор класса
//********************************************************************
nvdedit::nvdedit(int xpnum, QWidget* parent) : QWidget(parent) {

QString str;  
QFont font;
QFont oldfont;
QFont labelfont;

pnum=xpnum;

// Локальная копия данных раздела
data=new uint8_t[ptable->psize(pnum)];
plen=ptable->psize(pnum);
memcpy(data,ptable->iptr(pnum),plen);

// заголовок раздела
memcpy(&hdr,data,sizeof(hdr));

// тип файла
if (hdr.nv_bin.off == sizeof(hdr)) filetype=2; // полный заголовок - новые чипсеты
   else filetype=1;   // частичный заголовок - старые чипсеты
    
// компоненты раздела
//  nv.bin
nvpart=new uint8_t[hdr.nv_bin.len];
memcpy(nvpart,data+hdr.nv_bin.off,hdr.nv_bin.len);
// основной xml
if ((hdr.xnv_xml).magic == NV_FILE_MAGIC) {
 xmlpart=new uint8_t[hdr.xnv_xml.len];
 memcpy(xmlpart,data+hdr.xnv_xml.off,hdr.xnv_xml.len);
}
// дополнительный xml
if (hdr.cust_xml.magic == NV_FILE_MAGIC) {
 custxmlpart=new uint8_t[hdr.cust_xml.len];
 memcpy(custxmlpart,data+hdr.cust_xml.off,hdr.cust_xml.len);
}
// xml map
if (hdr.xnv_map.magic == NV_FILE_MAGIC) {
 xmlmap=new uint8_t[hdr.xnv_map.len];
 memcpy(xmlmap,data+hdr.xnv_map.off,hdr.xnv_map.len);
}


// Вертикальный компоновщик
vlm=new QVBoxLayout(this);

// Вынимаем текущие параметры шрифта меток 
font=QApplication::font("QLabel");
oldfont=font;

// Заголовок панели
font.setPointSize(font.pointSize()+7);
font.setBold(true);
hdrlabel=new QLabel("Редактор раздела NVDLOAD",this);
hdrlabel->setFont(font);
hdrlabel->setStyleSheet("QLabel { color : green; }");
vlm->addWidget(hdrlabel,0,Qt::AlignHCenter);

// Увеличиваем шрифт по умолчанию на 2 пункта
labelfont=oldfont;
labelfont.setPointSize(labelfont.pointSize()+2);

// Тип файла
if (filetype == 1) str = "Тип структуры NVDLOAD: 1 (чипсет V7R11 и более старые)";
else str = "Тип структуры NVDLOAD: 2 (чипсет V7R22 и более новые)";
hdrlabel=new QLabel(str,this);
hdrlabel->setFont(labelfont);
hdrlabel->setStyleSheet("QLabel { color : blue; }");
vlm->addWidget(hdrlabel);
vlm->addStretch(1);

// Компоновщик списка компонентов
lcomp=new QGridLayout(0);
lcomp->setVerticalSpacing(15);
vlm->addLayout(lcomp);

// заголовок таблицы
font=oldfont;
font.setPointSize(font.pointSize()+3);
font.setBold(true);

comphdr1=new QLabel("Компонент  ",this);
comphdr1->setFont(font);
comphdr1->setStyleSheet("QLabel { color : red; }");
lcomp->addWidget(comphdr1,0,0);

comphdr2=new QLabel("Размер",this);
comphdr2->setFont(font);
comphdr2->setStyleSheet("QLabel { color : orange; }");
lcomp->addWidget(comphdr2,0,1);

comphdr3=new QLabel("Команды",this);
comphdr3->setFont(font);
comphdr3->setStyleSheet("QLabel { color : green; }");
lcomp->addWidget(comphdr3,0,2,1,2,Qt::AlignHCenter);

// имена компонентов
name1=new QLabel("NVIMG",this);
name1->setFont(labelfont);
lcomp->addWidget(name1,1,0);

name2=new QLabel("Base XML",this);
name2->setFont(labelfont);
lcomp->addWidget(name2,2,0);

name3=new QLabel("Custom XML",this);
name3->setFont(labelfont);
lcomp->addWidget(name3,3,0);

name4=new QLabel("XML MAP",this);
name4->setFont(labelfont);
lcomp->addWidget(name4,4,0);

// размеры компонентов
str.sprintf("%i",hdr.nv_bin.len);
size1=new QLabel(str,this);
size1->setFont(labelfont);
lcomp->addWidget(size1,1,1,Qt::AlignHCenter);

str.sprintf("%i",hdr.xnv_xml.len);
size2=new QLabel(str,this);
size2->setFont(labelfont);
lcomp->addWidget(size2,2,1,Qt::AlignHCenter);

str.sprintf("%i",hdr.cust_xml.len);
size3=new QLabel(str,this);
size3->setFont(labelfont);
lcomp->addWidget(size3,3,1,Qt::AlignHCenter);

str.sprintf("%i",hdr.xnv_map.len);
size3=new QLabel(str,this);
size3->setFont(labelfont);
lcomp->addWidget(size3,4,1,Qt::AlignHCenter);


// кнопки извлечения 
extr1=new QPushButton("Извлечь",this);
connect(extr1,SIGNAL(clicked()),this,SLOT(extract1()));
lcomp->addWidget(extr1,1,2);

if (hdr.xnv_xml.len != 0) {
 extr2=new QPushButton("Извлечь",this);
 connect(extr2,SIGNAL(clicked()),this,SLOT(extract2()));
 lcomp->addWidget(extr2,2,2);
}

if (hdr.cust_xml.len != 0) {
 extr3=new QPushButton("Извлечь",this);
 connect(extr3,SIGNAL(clicked()),this,SLOT(extract3()));
 lcomp->addWidget(extr3,3,2);
}

if (hdr.xnv_map.len != 0) {
 extr4=new QPushButton("Извлечь",this);
 connect(extr4,SIGNAL(clicked()),this,SLOT(extract4()));
 lcomp->addWidget(extr4,4,2);
}

// кнопки замены
repl1=new QPushButton("Заменить",this);
connect(repl1,SIGNAL(clicked()),this,SLOT(replace1()));
lcomp->addWidget(repl1,1,3);

if (hdr.xnv_xml.len != 0) {
 repl2=new QPushButton("Заменить",this);
 connect(repl2,SIGNAL(clicked()),this,SLOT(replace2()));
 lcomp->addWidget(repl2,2,3);
}

if (hdr.cust_xml.len != 0) {
 repl3=new QPushButton("Заменить",this);
 connect(repl3,SIGNAL(clicked()),this,SLOT(replace3()));
 lcomp->addWidget(repl3,3,3);
}

if (hdr.xnv_map.len != 0) {
 repl4=new QPushButton("Заменить",this);
 connect(repl4,SIGNAL(clicked()),this,SLOT(replace4()));
 lcomp->addWidget(repl4,4,3);
}

// правая распорка
rspacer=new QSpacerItem(100,10,QSizePolicy::Expanding);
lcomp->addItem(rspacer,1,4);

vlm->addStretch(7);
}

//********************************************************************
//* Деструктор класса
//********************************************************************
nvdedit::~nvdedit() {

QMessageBox::StandardButton reply;
QString cmd;


// проверяем, изменились ли данные
if ((ptable->psize(pnum) != plen) || (memcmp(data,ptable->iptr(pnum),plen) != 0)) {
  reply=QMessageBox::warning(this,"Запись раздела","Содержимое раздела изменено, сохранить?",QMessageBox::Ok | QMessageBox::Cancel);
  if (reply == QMessageBox::Ok) {
    ptable->replace(pnum,data,plen+128);
  }
}  
delete data;
delete nvpart;
delete xmlpart;
delete custxmlpart;
delete xmlmap;

}



//********************************************************************
//* Извлечение компонентов
//*   0 - NVIMG
//*   1 - Base XML
//*   2 - Custom XML
//*   3 - XNV MAP
//********************************************************************
void nvdedit::extractor(int type) {

// имена файлов по умолчанию
char* compnames[4]= {
  "nvimg.nvm",
  "base.xml",
  "custom.xml",
  "xnvmap.bin"
};  
uint32_t start=0,len=0;

QString filename=compnames[type];

switch(type) {
  case 0:
    start=hdr.nv_bin.off;
    len=hdr.nv_bin.len;
    break;
    
  case 1:
    start=hdr.xnv_xml.off;
    len=hdr.xnv_xml.len;
    break;
    
  case 2:
    start=hdr.cust_xml.off;
    len=hdr.cust_xml.len;
    break;
    
  case 3:
    start=hdr.xnv_map.off;
    len=hdr.xnv_map.len;
    break;
}   

filename=QFileDialog::getSaveFileName(this,"Имя сохраняемого файла",filename,"All files (*.*)");
if (filename.isEmpty()) return;

QFile out(filename,this);
if (!out.open(QIODevice::WriteOnly)) {
    QMessageBox::critical(0,"Ошибка","Ошибка создания файла");
    return;
}
out.write((char*)(data+start),len);
out.close();
}


//********************************************************************
//* Слоты для извлечения образов компонентов
//********************************************************************
void nvdedit::extract1() { extractor(0); }
void nvdedit::extract2() { extractor(1); }
void nvdedit::extract3() { extractor(2); }
void nvdedit::extract4() { extractor(3); }

/*
//********************************************************************
//* Замена компонентов
//*   0 - ядро
//*   1 - рамдиск 1
//*   2 - рамдиск 2
//********************************************************************
void nvdedit::replacer(int type) {

QString filename="";
uint32_t kernelsize,r1size,r2size;
uint32_t bound_filesize, fsize;
uint32_t totalsize;
uint32_t pagesize=hdr->page_size;
uint8_t* newlocaldata;
uint8_t* srcptr;
uint8_t* dstptr;

// размеры компонентов, выравненные на границу страниц флешки
kernelsize=(hdr->kernel_size+pagesize-1)/pagesize*pagesize;
r1size=(hdr->ramdisk_size+pagesize-1)/pagesize*pagesize;
r2size=(hdr->second_size+pagesize-1)/pagesize*pagesize;

// выбор файла
filename=QFileDialog::getOpenFileName(this,"Имя файла",filename,"All files (*.*)");
if (filename.isEmpty()) return;

QFile out(filename,this);
if (!out.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(0,"Ошибка","Ошибка чтения файла");
    return;
}

// Читаем образ компонента из файла
fsize=out.size();
bound_filesize=(fsize+pagesize-1)/pagesize*pagesize; // округленный до страницы вверх
char* fbuf=new char[bound_filesize]; // файловый буфер
bzero(fbuf,bound_filesize);
out.read(fbuf,fsize);
out.close();

// Вычисляем новый размер раздела
totalsize=pagesize;
if (type == 0) totalsize+=bound_filesize;  else totalsize+=kernelsize;
if (type == 1) totalsize+=bound_filesize;  else totalsize+=r1size;
if (type == 2) totalsize+=bound_filesize;  else totalsize+=r2size;

// Выделяем память под новый образ раздела
newlocaldata=new uint8_t[totalsize+128];

// копируем хуавеевский и андроиндый заголовок
memcpy(newlocaldata,localdata,128+pagesize);
// перенастраиваем указатель на андроидный заголовок
hdr=(struct boot_img_hdr*)(newlocaldata+128);

// настраиваем указатели источника-приемника
srcptr=localdata+pagesize+128;
dstptr=newlocaldata+pagesize+128;

// копируем разделы
//------------------------
// ядро
if (type == 0) {
  memcpy(dstptr,fbuf,bound_filesize);
  srcptr+=kernelsize;
  dstptr+=bound_filesize;
  hdr->kernel_size=fsize;
}  
else {
  memcpy(dstptr,srcptr,kernelsize);
  srcptr+=kernelsize;
  dstptr+=kernelsize;
}

// рамдиск 1
if (type == 1) {
  memcpy(dstptr,fbuf,bound_filesize);
  srcptr+=r1size;
  dstptr+=bound_filesize;
  hdr->ramdisk_size=fsize;
  
}  
else {
  memcpy(dstptr,srcptr,r1size);
  srcptr+=r1size;
  dstptr+=r1size;
}

// рамдиск 2
if (type == 2) {
  memcpy(dstptr,fbuf,bound_filesize);
  srcptr+=r2size;
  dstptr+=bound_filesize;
  hdr->second_size=fsize;
}  
else {
  memcpy(dstptr,srcptr,r2size);
  srcptr+=r2size;
  dstptr+=r2size;
}

// удаляем файловый буфер
delete fbuf;
// Удаляем старый буфер и кладем на его место новый

delete localdata;
localdata=newlocaldata;
plen=totalsize;

}

//********************************************************************
//* Слоты для замены образов компонентов
//********************************************************************
void nvdedit::kreplace() { replacer(0); }
void nvdedit::r1replace() { replacer(1); }
void nvdedit::r2replace() { replacer(2); }
*/