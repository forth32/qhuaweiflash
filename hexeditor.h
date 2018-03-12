#ifndef __HEXEDITOR_H
#define __HEXEDITOR_H

#include <QtWidgets>
#include "hexeditor/qhexedit.h"

class hexeditor : public QWidget {

Q_OBJECT
  
QByteArray hexcup;
QVBoxLayout* lm;

public:

QHexEdit* dhex;

hexeditor(char* data, uint32_t len, QWidget* parent);
~hexeditor();

};


#endif // __HEXEDITOR_H