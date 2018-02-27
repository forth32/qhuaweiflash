#include <QtWidgets>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <termios.h>
#include <unistd.h>

#include "sio.h"
#include "usbloader.h"

// указатель на открытый последовательный порт
extern int siofd; // fd для работы с Последовательным портом


//*************************************************
//* Рассчет контрольной суммы командного пакета
//*************************************************
void csum(unsigned char* buf, uint32_t len) {

unsigned  int i,c,csum=0;

unsigned int cconst[]={0,0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF};

for (i=0;i<len;i++) {
  c=(buf[i]&0xff);
  csum=((csum<<4)&0xffff)^cconst[(c>>4)^(csum>>12)];
  csum=((csum<<4)&0xffff)^cconst[(c&0xf)^(csum>>12)];
}  
buf[len]=(csum>>8)&0xff;
buf[len+1]=csum&0xff;
  
}

//*************************************************
//*   Отсылка командного пакета модему
//*************************************************
int sendcmd(void* srcbuf, int len) {

unsigned char replybuf[1024];
unsigned char cmdbuf[2048];
unsigned int replylen;

// локальная копия командного буфера
memcpy(cmdbuf,srcbuf,len);

// добавляем в нее контрольную сумму
csum(cmdbuf,len);

// отсылка команды
write(siofd,cmdbuf,len+2);  
tcdrain(siofd);

// читаем ответ
replylen=read(siofd,replybuf,1024);

if (replylen == 0) return 0;     // пустой ответ
if (replybuf[0] == 0xaa) return 1; // правильный ответ
return 0;
}

//*************************************
//* Поиск linux-ядра в образе раздела
//*************************************
int locate_kernel(uint8_t* pbuf, uint32_t size) {
  
int off;

for(off=(size-8);off>0;off--) {
  if (strncmp((char*)(pbuf+off),"ANDROID!",8) == 0) return off;
}
return 0;
}

//*********************************************
//* Поиск таблицы разделов в загрузчике 
//*********************************************
uint32_t find_ptable(uint8_t* buf, uint32_t size) {

// сигнатура заголовка таблицы  
const uint8_t headmagic[16]={0x70, 0x54, 0x61, 0x62, 0x6c, 0x65, 0x48, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};  
uint32_t off;

for(off=0;off<(size-16);off+=4) {
  if (memcmp(buf+off,headmagic,16) == 0)   return off;
}
return 0;
}


//***************************************
//* Патч таблицы разделов
//***************************************
int ptable_patch(char* filename, uint8_t* pbuf[], struct lhead* part) {

FILE* in;
uint32_t fsize,ptoff;
char ptbuf[0x800];

in=fopen(filename,"r");
if (in == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка открытия файла");
  return 0;
}

// загружаем файл в буфер
fsize=fread(ptbuf,1,0x800,in);
fclose(in);
if (fsize != 0x800) {
  QMessageBox::critical(0,"Ошибка","Слишком короткий файл");
  return 0;
}  
if (strncmp((char*)ptbuf,"pTableHead",10) != 0) {
  QMessageBox::critical(0,"Ошибка","Файл не является таблицей разделов");
  return 0;
}  

// ищем таблицу разделов внутри загрузчика
ptoff=find_ptable(pbuf[1], part[1].size);
if (ptoff == 0) {
  QMessageBox::critical(0,"Ошибка","В загрузчике на найдена встроенная таблица разделов");
  return 0;
}  
// замещаем таблицу разделов
memcpy(pbuf[1]+ptoff,ptbuf,0x800);
return 1;
}



//***************************************
//* Выбор файла загрузчика
//***************************************
void usbldialog::browse() {

QString name;  
name=QFileDialog::getOpenFileName(this,"Выбор файла загрузчика",".","usbloader (*.bin);;All files (*.*)");
fname->setText(name);
}

//***************************************
//* Выбор файла таблицы разделов
//***************************************
void usbldialog::ptbrowse() {

QString name;  
name=QFileDialog::getOpenFileName(this,"Выбор файла таблицы разделов",".","usbloader (*.bin);;All files (*.*)");
ptfname->setText(name);
}


//***************************************
// fastboot-патч
//***************************************
int fastboot_only(uint8_t* pbuf[], struct lhead* part) {

int koff;  // смещение до ANDROID-заголовка

koff=locate_kernel(pbuf[1],part[1].size);
if (koff != 0) {
      *(pbuf[1]+koff)=0x55; // патч сигнатуры
      part[1].size=koff+8; // обрезаем раздел до начала ядра
      return 1;
}

QMessageBox::critical(0,"Ошибка"," В загрузчике нет ANDROID-компонента - fastboot-загрузка невозможна");
return 0;
  
}


//***************************************
//* Отправка заголовка компонента
//***************************************
int start_part(uint32_t size,uint32_t adr,uint8_t lmode) {
  
struct __attribute__ ((__packed__)) {
  uint8_t cmd[3]={0xfe,0, 0xff};
  uint8_t lmode;
  uint32_t size;
  uint32_t adr;
}  cmdhead;

cmdhead.size=htonl(size);
cmdhead.adr=htonl(adr);
cmdhead.lmode=lmode;
  
return sendcmd(&cmdhead,sizeof(cmdhead));
}  
  
    
//***************************************
//* Отправка пакета данных
//***************************************
int send_data_packet(uint32_t pktcount, uint8_t* databuf, uint32_t datasize) {

// образ пакета
struct __attribute__ ((__packed__)) {
 uint8_t cmd=0xda; 
 uint8_t count;
 uint8_t rcount;
 uint8_t data[2048];
} cmddata;
  
cmddata.count=pktcount&0xff;
cmddata.rcount=(~pktcount)&0xff;
memcpy(cmddata.data,databuf,datasize);

return sendcmd(&cmddata,datasize+3);
}  


//***************************************
//* Закрытие потока данных компонента
//***************************************
int close_part(uint32_t pktcount) {

struct __attribute__ ((__packed__)) {
 uint8_t cmd=0xed;
 uint8_t count;
 uint8_t rcount;
} cmdeod; 
  
// Фрмируем пакет конца данных
cmdeod.count=pktcount&0xff;;
cmdeod.rcount=(~pktcount)&0xff;

return sendcmd(&cmdeod,sizeof(cmdeod));
}

//***************************************
//* Запуск загрузки
//***************************************
void usbdload() {

// хранилище каталога компонентов загрузчика
struct lhead part[5];

// массив буферов для загрузки компонентов
uint8_t* pbuf[5]={0,0,0,0,0};

uint16_t numparts; // число компонентов для загрузки
  
uint32_t bl,datasize,pktcount;
uint32_t adr,i,fsize;
uint8_t c;
int32_t res;
int32_t pflag,fflag;
char filename[200];
char ptfilename[200];

FILE* in;

usbldialog* qd=new usbldialog;
QVBoxLayout* vl=new QVBoxLayout(qd);

QFont font;
font.setPointSize(17);
font.setBold(true);
font.setWeight(75);

QLabel* lbl1=new QLabel("USB BOOT");
lbl1->setFont(font);
lbl1->setScaledContents(true);
lbl1->setStyleSheet("QLabel { color : blue; }");

vl->addWidget(lbl1,4,Qt::AlignHCenter);

// вложенный lm для файлселекторов
QGridLayout* gvl=new QGridLayout(0);
vl->addLayout(gvl);

QLabel* lbl2=new QLabel("usbloader:");
gvl->addWidget(lbl2,0,0);

qd->fname=new QLineEdit(qd);
gvl->addWidget(qd->fname,0,1);

QToolButton* fselector = new QToolButton(qd);
gvl->addWidget(fselector,0,2);

QLabel* lbl3=new QLabel("Таблица разделов:");
gvl->addWidget(lbl2,1,0);

qd->ptfname=new QLineEdit(qd);
gvl->addWidget(qd->ptfname,1,1);

QToolButton* ptselector = new QToolButton(qd);
gvl->addWidget(ptselector,1,2);

// кнопки выбора режима загрузки
QCheckBox* fbflag = new QCheckBox("Загрузка в режиме FASTBOOT",qd);
vl->addWidget(fbflag);

QCheckBox* patchflag= new QCheckBox("Отключить патч eraseall (ОПАСНО!!!)",qd);
vl->addWidget(patchflag);

QDialogButtonBox* buttonBox = new QDialogButtonBox(qd);
buttonBox->setOrientation(Qt::Horizontal);
buttonBox->addButton("Отмена",QDialogButtonBox::RejectRole);
buttonBox->addButton("Загрузка",QDialogButtonBox::AcceptRole);
vl->addWidget(buttonBox,10,Qt::AlignHCenter);

QObject::connect(buttonBox, SIGNAL(accepted()), qd, SLOT(accept()));
QObject::connect(buttonBox, SIGNAL(rejected()), qd, SLOT(reject()));
QObject::connect(fselector, SIGNAL(triggered()), qd, SLOT(browse()));
QObject::connect(ptselector, SIGNAL(triggered()), qd, SLOT(ptbrowse()));

// Запускаем диалог
res=qd->exec();

// вынимаем данные из диалога
fflag=fbflag->isChecked();
pflag=patchflag->isChecked();
strcpy(filename,qd->fname->displayText().toLocal8Bit());
strcpy(ptfilename,qd->ptfname->displayText().toLocal8Bit());

// удаляем панель диалога
delete lbl1;
delete lbl2;
delete lbl3;
delete patchflag;
delete fbflag;
delete ptselector;
delete fselector;
delete gvl;
delete vl;
delete qd;

if (res != QDialog::Accepted) return;

//--------- Чтение загрузчика в память ---------------

// открываем файл загрузчика
in=fopen(filename,"r");
if (in == 0) {
  QMessageBox::critical(0,"Ошибка","Ошибка открыти файла");
  return;
}  
  

// Прверяем сигнатуру usloader
fread(&i,1,4,in);
if (i != 0x20000) {
  QMessageBox::critical(0,"Ошибка","Файл не является загрузчиком usbloader");
  fclose(in);
  return;
}  

// читаем заголовок загрузчика
fseek(in,36,SEEK_SET); // начало каталога компонентов в файле
fread(&part,sizeof(part),1,in);

// Ищем конец каталога компонентов
for (i=0;i<5;i++) {
  if (part[i].lmode == 0) break;
}
numparts=i;

// Загружаем компоненты в память
for(i=0;i<numparts;i++) {
 // встаем на начало образа компонента
 fseek(in,part[i].offset,SEEK_SET);
 // освобождаем предыдущий распределенный буфер
 if (pbuf[i] != 0) {
   free(pbuf[i]);
   pbuf[i]=0;
 }  
 // читаем в буфер весь компонент
 pbuf[i]=(uint8_t*)malloc(part[i].size);
 fsize=fread(pbuf[i],1,part[i].size,in);
 if (part[i].size != fsize) {
      QMessageBox::critical(0,"Ошибка","Неожиданный конец файла");
      fclose(in);
      return;
 }  
}

fclose(in);

// делаем fastboot-патч
if (fflag) {
  if (!fastboot_only(pbuf,part)) return;
}  

// Замещаем таблицу разделов
if (strlen(ptfilename) != 0) {
 if (!ptable_patch(ptfilename, pbuf, part)) return;
} 

//-------------------------------------------------------------------  
// Настройка SIO
if (!open_port())  {
  QMessageBox::critical(0,"Ошибка","Последовательный порт не открывается");
  return;
}  


// Проверяем загрузочный порт
c=0;
write(siofd,"A",1);   // отправляем произвольный байт в порт
bl=read(siofd,&c,1);
// ответ должен быть U (0x55)
if (c != 0x55) {
  QMessageBox::critical(0,"Ошибка","Последовательный порт не находится в режиме USB Boot");
  close_port();
  return;
}  


// Формируем панель индикаторов
QDialog* ind=new QDialog;
QFormLayout* lmf=new QFormLayout(ind);

QProgressBar* partbar = new QProgressBar(ind);
partbar->setValue(0);
lmf->addRow("Раздел:",partbar);

QProgressBar* totalbar = new QProgressBar(ind);
totalbar->setValue(0);
lmf->addRow("Всего:",totalbar);

ind->show();

// главный цикл загрузки - загружаем все блоки, найденные в заголовке

for(bl=0;bl<numparts;bl++) {

 // Прогрессбар компонентов
 totalbar->setValue(bl*100/numparts);
 QCoreApplication::processEvents();
  
 // стартовый пакет  
 if (!start_part(part[bl].size,part[bl].adr,part[bl].lmode)) {
   QMessageBox::critical(0,"Ошибка","Модем отверг заголовок компонента");
   goto leave;
 }  

  // Цикл поблочной загрузки данных
  datasize=1024;
  pktcount=1;
  for(adr=0;adr<part[bl].size;adr+=1024) {
    // проверка на последний блок компонента
    if ((adr+1024)>=part[bl].size) datasize=part[bl].size-adr; 
     
    // обновляем прогрессбар блоков 
    partbar->setValue(adr*100/part[bl].size);
    QCoreApplication::processEvents();
    
    if (!send_data_packet(pktcount++,(uint8_t*)(pbuf[bl]+adr),datasize)) {
      QMessageBox::critical(0,"Ошибка","Модем отверг пакет данных");
      goto leave;
    }  
  }


  if (!close_part(pktcount)) {
      QMessageBox::critical(0,"Ошибка","Модем отверг команду окончания компонента");
      goto leave;
    }  
} 

totalbar->setValue(100);
partbar->setValue(100);
QCoreApplication::processEvents();
      
QMessageBox::information(0,"ОК","Загрузка окончена");

leave:
close_port();
delete totalbar;
delete partbar;
delete lmf;
delete ind;

}

  