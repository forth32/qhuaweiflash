#include <QtWidgets>
#include "ui_flasher.h"

class flasher: public QDialog, public Ui_Flasher {
      Q_OBJECT
   void leave();   
   public:
      flasher(QWidget *parent = 0);
    public slots: 
       int exec();
};

 