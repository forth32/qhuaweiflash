// HEX-редактор образов разделов
#include "hexeditor.h"

//********************************************************************
//* Конструктор класса
//********************************************************************
hexeditor::hexeditor(char* data, uint32_t len, QMenuBar* mbar, QStatusBar* sbar, QWidget* parent) : QWidget(parent) {
  
int fontsize;  

// локальные копии указателей на элементы главного окна
menubar=mbar;
statusbar=sbar;

// Открываем доступ к конфигу
hconfig=new QSettings("forth32","qhuaweiflash",this);

// Создаем редактор
dhex=new QHexEdit(this);

// Установка размера шрифта
fontsize=hconfig->value("/config/hexfontsize").toInt();
if (fontsize>6) {
  font=dhex->font();
  font.setPointSize(fontsize);
  dhex->setFont(font);
}  

// Настройка внешнего вида редактора
dhex->setAddressWidth(6);
dhex->setOverwriteMode(true);
dhex->setHexCaps(true);
dhex->setHighlighting(true);

// Загрузка данных в редактор
hexcup.setRawData(data,len);
dhex->setData(hexcup);

dhex->setCursorPosition(0);
dhex->show();
dhex->setReadOnly(true);
// Компоновка окна

lm=new QVBoxLayout(this);
lm->addWidget(dhex);

// меню редактора
menu_edit = new QMenu("HEX-Редактор",menubar);
menubar->addMenu(menu_edit);

// меню undo-redo
menu_undo=menu_edit->addAction("Отмена",dhex,SLOT(undo()),QKeySequence::Undo);
menu_redo=menu_edit->addAction("Повтор",dhex,SLOT(redo()),QKeySequence::Redo);
// Увеличение-Уменьшение шрифта
menu_enlarge_font=menu_edit->addAction("Увеличить шрифт",this,SLOT(EnlargeFont()),QKeySequence("Ctrl++"));
menu_reduce_font=menu_edit->addAction("Уменьшить шрифт",this,SLOT(ReduceFont()),QKeySequence("Ctrl+-"));

// подменю выбора ширины hex-редактора
hwidth = new QMenu("Байт в строке",this);
wsel=new QActionGroup(hwidth);
wsel->setExclusive(true);
w16=hwidth->addAction("16");
w16->setCheckable(true);
w16->setActionGroup(wsel);

w32=hwidth->addAction("32");
w32->setCheckable(true);
w32->setActionGroup(wsel);

w48=hwidth->addAction("48");
w48->setCheckable(true);
w48->setActionGroup(wsel);

w64=hwidth->addAction("64");
w64->setCheckable(true);
w64->setActionGroup(wsel);

// достаем значение из конфига
bpl=hconfig->value("/config/bpl").toInt();


// Устанавливаем текущее значение
switch(bpl) {
  case 32:
    w32->setChecked(true);
    break;
    
  case 64:
    w64->setChecked(true);
    break;
    
  case 48:
    w48->setChecked(true);
    break;
    
  default:
    w16->setChecked(true);
    bpl=16;
    break;
}    
dhex->setBytesPerLine(bpl);
menu_edit->addMenu(hwidth);

menu_ro=menu_edit->addAction("Только чтение",this,SLOT(ROswitch()),QKeySequence("Ctrl+e"));
menu_ro->setCheckable(true);
menu_ro->setChecked(true);

// Инофрмация для статусбара
roindicator=new QLabel("R/O",this);
statusbar->addWidget(roindicator);  

status_adr_info=new QLabel(this);
statusbar->addWidget(status_adr_info);  

// Сигналы и слоты
connect(wsel,SIGNAL(triggered(QAction*)),this,SLOT(WidthSelector(QAction*)));
connect(dhex,SIGNAL(currentAddressChanged(qint64)),this,SLOT(ShowAddres(qint64)));
connect(dhex,SIGNAL(dataChanged()),this,SLOT(dchook()));
}

//********************************************************************
//* Деструктор класса
//********************************************************************
hexeditor::~hexeditor() {

statusbar->removeWidget(status_adr_info);  
statusbar->removeWidget(roindicator);  

delete menu_edit;  
}

//********************************************************************
//* Выбор ширины редактора
//********************************************************************
void hexeditor::WidthSelector(QAction* sel) {
  
if (sel == w16)  bpl=16;
else if (sel == w32) bpl=32;
else if (sel == w48) bpl=48;
else if (sel == w64) bpl=64;
dhex->setBytesPerLine(bpl);
// сохраняем в конфиг
hconfig->setValue("/config/bpl",bpl);

}



//********************************************************************
//*  Вывод текущего адреса в статусбар 
//********************************************************************
void hexeditor::ShowAddres(qint64 adr) {

static QString adrstr;
QByteArray data;

data=dhex->dataAt(adr,1);

adrstr.sprintf("Позиция: %06llX   Байт:%02X",adr,(uint8_t)data.at(0));
status_adr_info->setText(adrstr);   
}
    
//********************************************************************
//* Увеличение размера шрифта
//********************************************************************
void hexeditor::EnlargeFont() {ChangeFont(1); }

//********************************************************************
//* Уменьшение размера шрифта
//********************************************************************
void hexeditor::ReduceFont() { ChangeFont(-1); }
   
//********************************************************************
//* Изменение размера шрифта
//********************************************************************
void hexeditor::ChangeFont(int delta) {
  
int fsize;  

font=dhex->font();
fsize=font.pointSize();
fsize+=delta;
if ((fsize<6) || (fsize>25)) return; // пределы изменения размера шрифта

if (fsize == -1) return;
font.setPointSize(fsize);
dhex->setFont(font);
dhex->repaint(0,0,-1,-1);

// сохраняем размер шрифта в конфиг
hconfig->setValue("/config/hexfontsize",fsize);

}
   
//********************************************************************
//*  Переключатель чтения-записи
//********************************************************************
void hexeditor::ROswitch() {
  
readonly=menu_ro->isChecked(); 
if (readonly) roindicator->setText("R/O");
else roindicator->setText("R/W");
dhex->setReadOnly(readonly);  
dhex->ensureVisible();
}