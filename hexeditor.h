#ifndef __HEXEDITOR_H
#define __HEXEDITOR_H

#include <QtWidgets>
#include "hexedit2/qhexedit.h"

class hexeditor : public QWidget {

Q_OBJECT
  
QByteArray hexcup;
QVBoxLayout* lm;

QSettings* hconfig;

QMenuBar* menubar;
QStatusBar* statusbar;
QMenu* menu_edit;

QMenu* hwidth; // ширина редактора
QAction* w16;
QAction* w32;
QAction* w48;
QAction* w64;
QActionGroup* wsel;
QAction* menu_undo;
QAction* menu_redo;
QAction* menu_enlarge_font;
QAction* menu_reduce_font;


int bpl=16; // ширина строки редактора в байтах
QFont font;

QLabel* status_adr_info; // отображение адреса в статусбаре

public:

QHexEdit* dhex;

hexeditor(char* data, uint32_t len, QMenuBar* mbar, QStatusBar* sbar, QWidget* parent);
~hexeditor();
void ChangeFont(int delta);

public slots:
void WidthSelector(QAction* sel);  
void ShowAddres(qint64 adr);
void EnlargeFont();
void ReduceFont();

};

#endif // __HEXEDITOR_H