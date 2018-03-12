// HEX-редактор образов разделов
#include "hexeditor.h"
#include "MainWindow.h"

//********************************************************************
//* Конструктор класса
//********************************************************************
hexeditor::hexeditor(char* data, uint32_t len, QWidget* parent) : QWidget(parent) {
  
dhex=new QHexEdit(this);
dhex->setAddressWidth(8);
dhex->setOverwriteMode(true);
hexcup.setRawData(data,len);
dhex->setData(hexcup);
dhex->setCursorPosition(0);
dhex->show();

lm=new QVBoxLayout(this);
lm->addWidget(dhex);

mw->menu_edit->setEnabled(true);
  
}

//********************************************************************
//* Деструктор класса
//********************************************************************
hexeditor::~hexeditor() {
mw->menu_edit->setEnabled(false);

}
