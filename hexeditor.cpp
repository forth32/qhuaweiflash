// HEX-редактор образов разделов
#include "hexeditor.h"
#include "MainWindow.h"

//********************************************************************
//* Конструктор класса
//********************************************************************
hexeditor::hexeditor(char* data, uint32_t len, QWidget* parent) : QWidget(parent) {
  
dhex=new QHexEdit(this);
dhex->setAddressWidth(6);
dhex->setOverwriteMode(true);
hexcup.setRawData(data,len);
dhex->setData(hexcup);
dhex->setCursorPosition(0);
dhex->setHexCaps(true);
dhex->setHighlighting(true);
dhex->show();

lm=new QVBoxLayout(this);
lm->addWidget(dhex);

// меню undo-redo
menu_undo=mw->menu_edit->addAction("Отмена",dhex,SLOT(undo()),QKeySequence::Undo);
menu_redo=mw->menu_edit->addAction("Повтор",dhex,SLOT(redo()),QKeySequence::Redo);

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
hconfig=new QSettings("forth32","qhuaweiflash",this);
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
mw->menu_edit->addMenu(hwidth);

mw->menu_edit->setEnabled(true);

// Инофрмация для статусбара
status_adr_info=new QLabel(this);
mw->statusBar()->addWidget(status_adr_info);  

// Сигналы и слоты
connect(wsel,SIGNAL(triggered(QAction*)),this,SLOT(WidthSelector(QAction*)));
connect(dhex,SIGNAL(currentAddressChanged(qint64)),this,SLOT(ShowAddres(qint64)));

QFont font=dhex->font();
qDebug() << font;

}

//********************************************************************
//* Деструктор класса
//********************************************************************
hexeditor::~hexeditor() {

mw->statusBar()->removeWidget(status_adr_info);  
mw->menu_edit->clear();  
mw->menu_edit->setEnabled(false);
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

adrstr.sprintf("Позиция: %06llX   Байт:%02x",adr,(uint8_t)data.at(0));
status_adr_info->setText(adrstr);   
}
    