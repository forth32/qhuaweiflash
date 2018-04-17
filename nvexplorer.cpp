//-------------- Редактор двоичных образов NVRAM ----------------------------
#include "nvexplorer.h"
#include "sio.h"

//**************************************************
//* Конструктор класса
//**************************************************
nvexplorer::nvexplorer(uint8_t* srcdata, uint32_t srclen) : QMainWindow() {
 
uint32_t pos;
uint32_t i; 

// сохраняем параметры буфера с данными
pdata=srcdata;
plen=srclen;

// настройки геометрии окна
show();  
setAttribute(Qt::WA_DeleteOnClose);

config=new QSettings("forth32","qhuaweiflash",this);
QRect rect=config->value("/config/NvExplorerRect").toRect();
if (rect != QRect(0,0,0,0)) setGeometry(rect);
// выводим окно на передний план
setFocus();
raise();
activateWindow();

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

// Загружаем заголовок nv

memcpy(&nvhd, pdata, sizeof(nvhd));

// if (nvhd.magicnum != FILE_MAGIC_NUM) {
//   QMessageBox::critical(0,"Ошибка","Ошибка структуры образа NVRAM - неправильная сигнатура заголовка");
//   delete this;
// }

// Определяем тип CRC
switch (nvhd.crcflag) {
  case 0:
    crcmode=0;
    break;
    
  case 1:  
    crcmode=1;
    break;
    
  case 8:
    crcmode=2;
    break;
    
  default:
    crcmode=-1;
    break;
    
}

//----- Читаем каталог файлов

pos=nvhd.ctrl_size; // смещение до начала данных (конец управляющих структур)

// размер описателя файла в образе nvram
uint32_t fcsize=sizeof(struct nv_file)-4;
// смещение до описателя текущего файла
uint32_t fsoffset;

// вынимаем все описатели файлов
for(i=0;i<nvhd.file_num;i++) {
 fsoffset=i*fcsize; 
 memcpy(&flist[i],pdata+nvhd.file_offset+fsoffset,fcsize); 
 // вычисляем смещение до данных файла
 flist[i].offset=pos;
 pos+=flist[i].size;
}

// получаем смещение до поля CRC
crcoff=pos;

//----- Читаем каталог ячеек
itemlist=new struct nv_item[nvhd.item_size];
memcpy(itemlist,pdata+nvhd.item_offset,nvhd.item_size);

// Создаем таблицу nvram
nvtable=new QTableWidget(nvhd.item_count,5,central);
vlm->addWidget(nvtable,2);

// заголовок таблицы
QStringList plst;
plst << "ID" << "Размер" <<"Компонент" <<"Имя" <<"Содержимое";
nvtable->setHorizontalHeaderLabels(plst);

// выводим список ячеек в таблицу
QTableWidgetItem* cell;
QString str;

for(uint32_t i=0;i<nvhd.item_count;i++) {
  // id ячейки
  str.setNum(itemlist[i].id);
  cell=new QTableWidgetItem(str);
  nvtable->setItem(i,0,cell);
  // размер ячейки
  str.setNum(itemlist[i].len);
  cell=new QTableWidgetItem(str);
  nvtable->setItem(i,1,cell);
  // компонент
  int fid=itemlist[i].file_id;
  str.sprintf("%1i:%s",fid,flist[fileidx(fid)].name);
  cell=new QTableWidgetItem(str);
  nvtable->setItem(i,2,cell);
  // имя
  str=find_desc(itemlist[i].id);
  cell=new QTableWidgetItem(str);
  nvtable->setItem(i,3,cell);
  
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
// пункты меню
menu_file->addAction(QIcon::fromTheme("document-save"),"Сохранить",this,SLOT(save_all()),QKeySequence::Save);
toolbar->addAction(QIcon::fromTheme("document-save"),"Сохранить",this,SLOT(save_all()));
menu_file->addSeparator();
menu_file->addAction("Выход",this,SLOT(close()),QKeySequence("Esc"));

toolbar->addSeparator();

menu_view->addAction(QIcon::fromTheme("zoom-in"),"Увеличить шрифт",nvtable,SLOT(zoomIn()),QKeySequence("Ctrl++"));
toolbar->addAction(QIcon::fromTheme("zoom-in"),"Увеличить шрифт",nvtable,SLOT(zoomIn()));
menu_view->addAction(QIcon::fromTheme("zoom-out"),"Уменьшить шрифт",nvtable,SLOT(zoomOut()),QKeySequence("Ctrl+-"));
toolbar->addAction(QIcon::fromTheme("zoom-out"),"Уменьшить шрифт",nvtable,SLOT(zoomOut()));
// // menu_view->addAction(QIcon::fromTheme("preferences-desktop-font"),"Шрифт...",this,SLOT(fontselector()));

}

//**********************************************************************
//*  ДЕструктор класса
//**********************************************************************
nvexplorer::~nvexplorer() {
QFont font;

// сохраняем размер шрифта
// font=ted->font();
// config->setValue("/config/EditorFont",font);

// геометрия главного окна
QRect rect=geometry();
config->setValue("/config/NvExplorerRect",rect);

delete [] itemlist;  
delete nvtable;
}

