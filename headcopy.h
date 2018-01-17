#include <QtWidgets>
#include "ui_headcopy.h"
#include "ptable.h"

extern ptable_list* ptable;  

class headcopy: public QDialog, public Ui_headcopy {
      Q_OBJECT

 public:
      headcopy(QWidget *parent = 0);
 public slots: 
      int exec();
};

void head_copy();