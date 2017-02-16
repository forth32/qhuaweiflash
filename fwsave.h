#include <QtWidgets>
#include "ui_fwsave.h"
#include "ptable.h"

extern ptable_list* ptable;  

class fwsave: public QDialog, public Ui_fwsave {
      Q_OBJECT
   void leave();   
   public:
      fwsave(QWidget *parent = 0);
    public slots: 
      void browse();
      int exec();
};

void fw_saver(); 