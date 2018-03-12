#ifndef __HEXEDITOR_H
#define __HEXEDITOR_H

#include <QtWidgets>
#include "hexedit2/qhexedit.h"

class hexeditor : public QWidget {

Q_OBJECT
  
QByteArray hexcup;
QVBoxLayout* lm;

QSettings* hconfig;

QMenu* hwidth; // ширина редактора
QAction* w16;
QAction* w32;
QAction* w48;
QAction* w64;
QActionGroup* wsel;
QAction* menu_undo;
QAction* menu_redo;

int bpl=16; // ширина строки редактора в байтах

public:

QHexEdit* dhex;

hexeditor(char* data, uint32_t len, QWidget* parent);
~hexeditor();

public slots:
void WidthSelector(QAction* sel);  

};

#endif // __HEXEDITOR_H