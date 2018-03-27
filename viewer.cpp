// просмотр и редактирование произвольных файлов 

#include "viewer.h"

//***********************************************************
//* Конструктор просмотрщика
//***********************************************************
viewer::viewer(cpfiledir* dfile, uint8_t rmode) : QMainWindow() {
  
QString title;
  
// настройки геометрии окна
show();  
setAttribute(Qt::WA_DeleteOnClose);

config=new QSettings("forth32","qhuaweiflash",this);
QRect rect=config->value("/config/EditorRect").toRect();
if (rect != QRect(0,0,0,0)) setGeometry(rect);
// выводим окно на передний план
setFocus();
raise();
activateWindow();

// сохраняем на будущее входные параметры  
fileptr=dfile;
readonly=rmode;

// копируем данные в локальный буфер
plen=fileptr->fsize();
pdata=new uint8_t[plen+1];
memcpy(pdata,fileptr->fdata(),plen);
pdata[plen]=0; // ограничитель строки

// заголовок окна
if (readonly) title="Просмотр - ";
else title="Редактирование - ";
title.append(fileptr->fname());
setWindowTitle(title);

// Главное меню
menubar = new QMenuBar(this);
setMenuBar(menubar);

menu_file = new QMenu("Файл",menubar);
menubar->addAction(menu_file->menuAction());

menu_edit = new QMenu("Правка",menubar);
menubar->addAction(menu_edit->menuAction());

menu_view = new QMenu("Вид",menubar);
menubar->addAction(menu_view->menuAction());

// пункты меню
menu_file->addAction("Выход",this,SLOT(close()),QKeySequence("Esc"));

// menu_view

// Центральный виджет
central=new QWidget(this);
setCentralWidget(central);

// основной компоновщик
vlm=new QVBoxLayout(central);

// текстовый редактор
ted=new QTextEdit(central);
ted->setReadOnly(readonly);
vlm->addWidget(ted,2);

// наполнение текстового редактора
textdata=(char*)pdata;
ted->append(textdata);

// слот модификации
connect(ted,SIGNAL(textChanged()),this,SLOT(setChanged()));

}

//***********************************************************
//* Деструктор просмотрщика
//***********************************************************
viewer::~viewer() {

QMessageBox::StandardButton reply;
QByteArray xdata;

// признак изменения данных
if (datachanged) {
  reply=QMessageBox::warning(this,"Запись файла","Содержимое файла изменено, сохранить?",QMessageBox::Ok | QMessageBox::Cancel);
  if (reply == QMessageBox::Ok) {
    // сохранение данных
    textdata=ted->toPlainText();
    xdata=textdata.toLocal8Bit();
    fileptr->replace_data((uint8_t*)xdata.data(),xdata.size());
    // вызываем сигнал- признак модификации
    emit changed();
  }
}  
  
// геометрия главного окна
QRect rect=geometry();
config->setValue("/config/EditorRect",rect);
delete config;

delete pdata;  
}


//***********************************************************
//* Вызов внешнего слота модификации
//***********************************************************
void viewer::setChanged() { 

QString str;

datachanged=true;
// рассоединяем сигнал - он нужен ровно один раз
disconnect(ted,SIGNAL(textChanged()),this,SLOT(setChanged()));
// добавляем звездочку в заголовок
str=windowTitle();
str.append(" *");
setWindowTitle(str);
}
