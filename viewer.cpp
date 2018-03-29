// просмотр и редактирование произвольных файлов 

#include "viewer.h"

//***********************************************************
//* Конструктор просмотрщика
//***********************************************************
viewer::viewer(cpfiledir* dfile, uint8_t rmode) : QMainWindow() {
  
QString title;
QFont font;
int fontsize;

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


// тулбар
toolbar=new QToolBar(this);
addToolBar(toolbar);

// Центральный виджет
central=new QWidget(this);
setCentralWidget(central);

// основной компоновщик
vlm=new QVBoxLayout(central);

// текстовый редактор
ted=new QTextEdit(central);
ted->setReadOnly(readonly);
vlm->addWidget(ted,2);

// шрифт редактора
font=ted->font();
fontsize=config->value("/config/EditorFontSize").toInt();
if (fontsize != 0) {
   font.setPointSize(fontsize);
   ted->setFont(font);
// ted->setFontPointSize(fontsize);
}  
// наполнение текстового редактора
textdata=(char*)pdata;
ted->append(textdata);

// пункты меню
menu_file->addAction(QIcon::fromTheme("document-save"),"Сохранить",this,SLOT(save_all()),QKeySequence::Save);
toolbar->addAction(QIcon::fromTheme("document-save"),"Сохранить",this,SLOT(save_all()));
menu_file->addSeparator();
menu_file->addAction("Выход",this,SLOT(close()),QKeySequence("Esc"));

toolbar->addSeparator();

if (!readonly) {
  menu_edit->addAction(QIcon::fromTheme("edit-undo"),"Отменить",ted,SLOT(undo()),QKeySequence::Undo);
  toolbar->addAction(QIcon::fromTheme("edit-undo"),"Отменить",ted,SLOT(undo()));
  menu_edit->addAction(QIcon::fromTheme("edit-redo"),"Повторить",ted,SLOT(redo()),QKeySequence::Redo);
  toolbar->addAction(QIcon::fromTheme("edit-redo"),"Повторить",ted,SLOT(redo()));
  menu_edit->addSeparator();
  menu_edit->addAction(QIcon::fromTheme("edit-cut"),"Вырезать",ted,SLOT(cut()),QKeySequence::Cut);
  toolbar->addAction(QIcon::fromTheme("edit-cut"),"Вырезать",ted,SLOT(cut()));
}
menu_edit->addAction(QIcon::fromTheme("edit-copy"),"Копировать",ted,SLOT(copy()),QKeySequence::Copy);
toolbar->addAction(QIcon::fromTheme("edit-copy"),"Копировать",ted,SLOT(copy()));

if (!readonly) {
  menu_edit->addAction(QIcon::fromTheme("edit-paste"),"Вставить",ted,SLOT(paste()),QKeySequence::Paste);
  toolbar->addAction(QIcon::fromTheme("edit-paste"),"Вставить",ted,SLOT(paste()));
  toolbar->addSeparator();
}
menu_edit->addAction(QIcon::fromTheme("edit-find"),"Найти...",this,SLOT(find()),QKeySequence::Find);
toolbar->addAction(QIcon::fromTheme("edit-find"),"Найти...",this,SLOT(find()));
menu_edit->addAction(QIcon::fromTheme("edit-find"),"Найти далее",this,SLOT(findnext()),QKeySequence::FindNext);


menu_view->addAction(QIcon::fromTheme("zoom-in"),"Увеличить шрифт",ted,SLOT(zoomIn()),QKeySequence("Ctrl++"));
toolbar->addAction(QIcon::fromTheme("zoom-in"),"Увеличить шрифт",ted,SLOT(zoomIn()));
menu_view->addAction(QIcon::fromTheme("zoom-out"),"Уменьшить шрифт",ted,SLOT(zoomOut()),QKeySequence("Ctrl+-"));
toolbar->addAction(QIcon::fromTheme("zoom-out"),"Уменьшить шрифт",ted,SLOT(zoomOut()));

// слот модификации
connect(ted,SIGNAL(textChanged()),this,SLOT(setChanged()));

ted->setFocus();
}

//***********************************************************
//* Деструктор просмотрщика
//***********************************************************
viewer::~viewer() {

QMessageBox::StandardButton reply;
QFont font;
int fontsize;

// сохраняем размер шрифта
font=ted->font();
fontsize=font.pointSize();
config->setValue("/config/EditorFontSize",fontsize);

// геометрия главного окна
QRect rect=geometry();
config->setValue("/config/EditorRect",rect);

// признак изменения данных
if (datachanged) {
  reply=QMessageBox::warning(this,"Запись файла","Содержимое файла изменено, сохранить?",QMessageBox::Ok | QMessageBox::Cancel);
  if (reply == QMessageBox::Ok) {
    // сохранение данных
    save_all();
  }
}  
  
delete config;
delete pdata;  
}

//***********************************************************
//* Сохранение данных в вектор файла
//***********************************************************
void viewer::save_all() {

QByteArray xdata;
QString str;
int pos;

textdata=ted->toPlainText();
xdata=textdata.toLocal8Bit();
fileptr->replace_data((uint8_t*)xdata.data(),xdata.size());
// вызываем сигнал- признак модификации
emit changed();

// удаляем звездочку из заголовка
str=windowTitle();
pos=str.indexOf('*');
if (pos != -1) {
  str.truncate(pos-1);
  setWindowTitle(str);
}  
// восстанавливаем обработчик модификации
datachanged=false;
connect(ted,SIGNAL(textChanged()),this,SLOT(setChanged()));

}

//***********************************************************
//* Поиск текста
//***********************************************************
void viewer::find() {

int res;  
  
QInputDialog* pd=new QInputDialog(this);  
res=pd->exec();
if (res == QDialog::Accepted) {
 findtext=pd->textValue();
 findnext();
}
delete pd;
}

//***********************************************************
//* Продолжение поиска текста
//***********************************************************
void viewer::findnext() {

int res;

if (findtext.size() == 0) return;
  res=ted->find(findtext);
  if (!res) {
    QMessageBox::information(0,"Информация","Текст не найден");
  } 
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
