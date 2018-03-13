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
vlm->addWidget(hdrlabel,0,Qt::AlignHCenter);

// Заголовок списка параметров
font=oldfont;
font.setPointSize(font.pointSize()+5);
font.setBold(true);
parmlabel=new QLabel("Параметры ядра",this);
parmlabel->setFont(font);
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
vlm->addWidget(complabel);

vlm->addStretch(1);
// Компоновщик списка компонентов
lcomp=new QGridLayout(0);
lcomp->setVerticalSpacing(9);
vlm->addLayout(lcomp);

kcomp=new QLabel("Kernel image  ",this);
kcomp->setFont(labelfont);
lcomp->addWidget(kcomp,0,0);

r1comp=new QLabel("Ramdisk1",this);
r1comp->setFont(labelfont);
lcomp->addWidget(r1comp,1,0);

r2comp=new QLabel("Ramdisk2",this);
r2comp->setFont(labelfont);
lcomp->addWidget(r2comp,2,0);

kext=new QPushButton("Извлечь",this);
lcomp->addWidget(kext,0,1);

r1ext=new QPushButton("Извлечь",this);
lcomp->addWidget(r1ext,1,1);

r2ext=new QPushButton("Извлечь",this);
lcomp->addWidget(r2ext,2,1);

krepl=new QPushButton("Заменить",this);
lcomp->addWidget(krepl,0,2);

r1repl=new QPushButton("Заменить",this);
lcomp->addWidget(r1repl,1,2);

r2repl=new QPushButton("Заменить",this);
lcomp->addWidget(r2repl,2,2);

// правая распорка
rspacer=new QSpacerItem(100,10,QSizePolicy::Expanding);
lcomp->addItem(rspacer,0,3);

vlm->addStretch(5);
}

//********************************************************************
//* Деструктор класса
//********************************************************************
kerneledit::~kerneledit() {

delete pdata;  
}
    