// редактор раздела kernel с ядром линукса
#include "kerneledit.h"
#include "MainWindow.h"
#include <string.h>

//********************************************************************
//* Конструктор класса
//********************************************************************
kerneledit::kerneledit(uint8_t* data, uint32_t len, QWidget* parent) : QWidget(parent) {

QString str;  
char cline[513];
QFont font;
QFont oldfont;
QFont labelfont;

// Локальная копия данных раздела
pdata=new uint8_t[len];
memcpy(pdata,data,len);
plen=len;

// указатель на заголовок раздела
hdr=(struct boot_img_hdr*)pdata;

// Вертикальный компоновщик
vlm=new QVBoxLayout(this);

// Вынимаем текущие параметры шрифта меток 
font=QApplication::font("QLabel");
oldfont=font;

// Заголовок панели
font.setPointSize(font.pointSize()+7);
font.setBold(true);
hdrlabel=new QLabel("Редактор раздела KERNEL",this);
hdrlabel->setFont(font);
hdrlabel->setStyleSheet("QLabel { color : green; }");
vlm->addWidget(hdrlabel,0,Qt::AlignHCenter);

// Заголовок списка параметров
font=oldfont;
font.setPointSize(font.pointSize()+5);
font.setBold(true);
parmlabel=new QLabel("Параметры ядра",this);
parmlabel->setFont(font);
parmlabel->setStyleSheet("QLabel { color : blue; }");
vlm->addWidget(parmlabel);
vlm->addStretch(1);

// Увеличиваем шрифт по умолчанию на 2 пункта
labelfont=oldfont;
labelfont.setPointSize(labelfont.pointSize()+2);

// Компоновщик строк с параметрами
flm=new QFormLayout(0);
vlm->addLayout(flm);

// Размер страницы
str.sprintf("%i",hdr->page_size);
pgslabel=new QLabel(str,this);
pgslabel->setFont(labelfont);
// pgslabel->setFont(labelfont);
flm->addRow("Размер страницы",pgslabel);
flm->labelForField(pgslabel)->setFont(labelfont);

// physical addr for kernel tags
str.sprintf("%08x",hdr->tags_addr);
tagslabel=new QLabel(str,this);
tagslabel->setFont(labelfont);
flm->addRow("physical addr for kernel tags",tagslabel);
flm->labelForField(tagslabel)->setFont(labelfont);

// device tree in bytes
str.sprintf("%08x",hdr->dt_size);
dtlabel=new QLabel(str);
dtlabel->setFont(labelfont);
flm->addRow("physical addr for kernel tags",dtlabel);
flm->labelForField(dtlabel)->setFont(labelfont);

// product name
// strncpy(cline,(char*)(hdr->name),BOOT_NAME_SIZE);
// str=cline;
// pname=new QLabel(str,this);
// flm->addRow("product name",pname);

// параметры загрузки
strncpy(cline,(char*)hdr->cmdline,BOOT_ARGS_SIZE);
str=cline;
cmdline=new QLineEdit(str,this);
flm->addRow("Параметры загрузки",cmdline);
flm->labelForField(cmdline)->setFont(labelfont);

vlm->addStretch(1);

// Заголовок редактора компонентов
complabel=new QLabel("Компоненты ядра",this);
complabel->setFont(font);
complabel->setStyleSheet("QLabel { color : blue; }");
vlm->addWidget(complabel);

vlm->addStretch(1);
// Компоновщик списка компонентов
lcomp=new QGridLayout(0);
lcomp->setVerticalSpacing(9);
vlm->addLayout(lcomp);

// 

// имена компонентов
kcomp=new QLabel("Kernel image  ",this);
kcomp->setFont(labelfont);
lcomp->addWidget(kcomp,0,0);

r1comp=new QLabel("Ramdisk1",this);
r1comp->setFont(labelfont);
lcomp->addWidget(r1comp,1,0);

r2comp=new QLabel("Ramdisk2",this);
r2comp->setFont(labelfont);
lcomp->addWidget(r2comp,2,0);

// адреса загрузки
str.sprintf("%08x",hdr->kernel_addr);
kaddr=new QLabel(str,this);
kaddr->setFont(labelfont);
lcomp->addWidget(kaddr,0,1);

str.sprintf("%08x",hdr->ramdisk_addr);
r1addr=new QLabel(str,this);
r1addr->setFont(labelfont);
lcomp->addWidget(r1addr,1,1);

str.sprintf("%08x",hdr->second_addr);
r2addr=new QLabel(str,this);
r2addr->setFont(labelfont);
lcomp->addWidget(r2addr,2,1);

// кнопки извлечения
kext=new QPushButton("Извлечь",this);
connect(kext,SIGNAL(clicked()),this,SLOT(kextract()));
lcomp->addWidget(kext,0,2);

r1ext=new QPushButton("Извлечь",this);
connect(r1ext,SIGNAL(clicked()),this,SLOT(r1extract()));
lcomp->addWidget(r1ext,1,2);

r2ext=new QPushButton("Извлечь",this);
connect(r2ext,SIGNAL(clicked()),this,SLOT(r2extract()));
lcomp->addWidget(r2ext,2,2);

// кнопки замены
krepl=new QPushButton("Заменить",this);
connect(krepl,SIGNAL(clicked()),this,SLOT(kreplace()));
lcomp->addWidget(krepl,0,3);

r1repl=new QPushButton("Заменить",this);
connect(r1repl,SIGNAL(clicked()),this,SLOT(r1replace()));
lcomp->addWidget(r1repl,1,3);

r2repl=new QPushButton("Заменить",this);
connect(r2repl,SIGNAL(clicked()),this,SLOT(r2replace()));
lcomp->addWidget(r2repl,2,3);

// правая распорка
rspacer=new QSpacerItem(100,10,QSizePolicy::Expanding);
lcomp->addItem(rspacer,0,4);

vlm->addStretch(7);
}

//********************************************************************
//* Деструктор класса
//********************************************************************
kerneledit::~kerneledit() {

delete pdata;  
}

//********************************************************************
//* Вычисление адреса компонента
//*   0 - ядро
//*   1 - рамдиск 1
//*   2 - рамдиск 2
//********************************************************************
void kerneledit::setup_adr(int type, uint32_t* adr, uint32_t* rlen, QString* filename=0) {
uint32_t start=0;
uint32_t size=0;

uint32_t kernelsize=(hdr->kernel_size+hdr->page_size-1)/hdr->page_size;
uint32_t ram1size=(hdr->ramdisk_size+hdr->page_size-1)/hdr->page_size;

switch(type) {
  case 0:
    start=1;
    size=hdr->kernel_size;
    if (filename != 0) *filename="vmlinuz.bin";
    break;
    
  case 1:
    start=kernelsize+1;
    size=hdr->ramdisk_size;
    if (filename != 0) *filename="ramdisk1.cpio.gz";
    break;
    
  case 2:
    start=kernelsize+ram1size+1;
    size=hdr->second_size;
    if (filename != 0) *filename="ramdisk2.cpio.gz";
    break;
}
// переводим из страниц в байты
start*=hdr->page_size;
*adr=start;
*rlen=size;
}


//********************************************************************
//* Извлечение компонентов
//*   0 - ядро
//*   1 - рамдиск 1
//*   2 - рамдиск 2
//********************************************************************
void kerneledit::extractor(int type) {

uint32_t start;
uint32_t size;
QString filename;

setup_adr(type,&start,&size,&filename);
filename=QFileDialog::getSaveFileName(this,"Имя сохраняемого файла",filename,"All files (*.*)");
if (filename.isEmpty()) return;

QFile out(filename,this);
if (!out.open(QIODevice::WriteOnly)) {
    QMessageBox::critical(0,"Ошибка","Ошибка создания файла");
    return;
}
out.write((char*)(pdata+start),size);
out.close();
}


//********************************************************************
//* Слоты для извлечения образов компонентов
//********************************************************************
void kerneledit::kextract() { extractor(0); }
void kerneledit::r1extract() { extractor(1); }
void kerneledit::r2extract() { extractor(2); }

//********************************************************************
//* Замена компонентов
//*   0 - ядро
//*   1 - рамдиск 1
//*   2 - рамдиск 2
//********************************************************************
void kerneledit::replacer(int type) {

QString filename="";
uint32_t kernelsize,r1size,r2size;
uint32_t bound_filesize, fsize;
uint32_t totalsize;
uint32_t pagesize=hdr->page_size;
uint8_t* newpdata;
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
newpdata=new uint8_t[totalsize];

// копируем заголовок и переустанавливаем указатель на него
memcpy(newpdata,pdata,pagesize);
hdr=(struct boot_img_hdr*)newpdata;

// настраиваем указатели источника-приемника
srcptr=pdata+pagesize;
dstptr=newpdata+pagesize;

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
if (type == 1) {
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

// Удаляем старый буфер и кладем на его место новый
delete fbuf;
delete pdata;
pdata=newpdata;
plen=totalsize;

}

//********************************************************************
//* Слоты для замены образов компонентов
//********************************************************************
void kerneledit::kreplace() { replacer(0); }
void kerneledit::r1replace() { replacer(1); }
void kerneledit::r2replace() { replacer(2); }
