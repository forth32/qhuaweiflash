#include <QtWidgets>
#include "ptable.h"

extern ptable_list* ptable;  

//****************************************************************
//* Класс диалогового окна
//****************************************************************
class fsdialog: public QDialog {
Q_OBJECT
public:
  fsdialog(): QDialog(0){};

public slots: 
  void browse();
  int exec();
};  

void fw_saver(int); 