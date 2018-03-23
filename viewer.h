// просмотр и редактирование произвольных файлов 

#ifndef __VIEWER_H
#define __VIEWER_H
#include <stdint.h>
#include <QtWidgets>
#include "cpfiledir.h"

//***********************************************************
//* Класс главного окна 
//***********************************************************
class viewer  : public QWidget {

Q_OBJECT

cpfiledir* fileptr;
bool readonly;

QVBoxLayout* vlm;
QTextEdit* ted;

QString textdata;

public:
  
viewer(cpfiledir* dfile, uint8_t rmode);  
~viewer();

  
};

#endif
