#include "hexfileviewer.h"

//***********************************************************
//* Конструктор HEX-просмотрщика
//***********************************************************
hexfileviewer::hexfileviewer(cpfiledir* dfile) : QMainWindow() {
  
QString title;


setAttribute(Qt::WA_DeleteOnClose);


// настройки геометрии окна
config=new QSettings("forth32","qhuaweiflash",this);
QRect rect=config->value("/config/HexFileEditorRect").toRect();
if (rect != QRect(0,0,0,0)) setGeometry(rect);
show();  

// выводим окно на передний план
setFocus();
raise();
activateWindow();

// сохраняем на будущее входные параметры  
fileptr=dfile;

// копируем данные в локальный буфер
plen=fileptr->fsize();
pdata=new uint8_t[plen];
memcpy(pdata,fileptr->fdata(),plen);

// заголовок окна
title="HEX-Просмотр - ";
title.append(fileptr->fname());
setWindowTitle(title);

// Главное меню
menubar = new QMenuBar(this);
setMenuBar(menubar);

menu_file = new QMenu("Файл",menubar);
menubar->addAction(menu_file->menuAction());

// Статусбар
statusbar = new QStatusBar(this);
setStatusBar(statusbar);

// Центральный виджет
central=new QWidget(this);
setCentralWidget(central);

// пункты меню
menu_file->addAction(QIcon::fromTheme("document-save"),"Сохранить",this,SLOT(save_all()),QKeySequence::Save);
menu_file->addSeparator();
menu_file->addAction("Выход",this,SLOT(close()),QKeySequence("Esc"));

// основной компоновщик
vlm=new QVBoxLayout(central);

// hex-редактор
hed=new hexeditor((char*)pdata,plen,menubar,statusbar,central);
vlm->addWidget(hed,2);

// слот модификации
connect(hed,SIGNAL(dataChanged()),this,SLOT(setChanged()));

hed->setFocus();
}

//***********************************************************
//* Деструктор просмотрщика
//***********************************************************
hexfileviewer::~hexfileviewer() {

QMessageBox::StandardButton reply;

// геометрия главного окна
QRect rect=geometry();
config->setValue("/config/HexFileEditorRect",rect);

// признак изменения данных
if (datachanged) {
  reply=QMessageBox::warning(this,"Запись файла","Содержимое файла изменено, сохранить?",QMessageBox::Ok | QMessageBox::Cancel);
  if (reply == QMessageBox::Ok) {
    // сохранение данных
    save_all();
  }
}  
delete hed;  
delete config;
delete pdata;  
}

//***********************************************************
//* Сохранение данных в вектор файла
//***********************************************************
void hexfileviewer::save_all() {

QByteArray xdata;
QString str;
int pos;

fileptr->replace_data((uint8_t*)pdata,plen);

// удаляем звездочку из заголовка
str=windowTitle();
pos=str.indexOf('*');
if (pos != -1) {
  str.truncate(pos-1);
  setWindowTitle(str);
}  
// вызываем сигнал- признак модификации
emit changed();

// восстанавливаем обработчик модификации
datachanged=false;
connect(hed,SIGNAL(dataChanged()),this,SLOT(setChanged()));

}

//***********************************************************
//* Вызов внешнего слота модификации
//***********************************************************
void hexfileviewer::setChanged() { 

QString str;

datachanged=true;
// рассоединяем сигнал - он нужен ровно один раз
disconnect(hed,SIGNAL(dataChanged()),this,SLOT(setChanged()));
// добавляем звездочку в заголовок
str=windowTitle();
str.append(" *");
setWindowTitle(str);
}
