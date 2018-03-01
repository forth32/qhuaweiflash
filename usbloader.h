#include <QtWidgets>

//****************************************************************
//* Класс диалогового окна
//****************************************************************
class usbldialog: public QDialog {

Q_OBJECT
public:
  QLineEdit* fname=0;
  QLineEdit* ptfname=0;
  
  // конструктор
  usbldialog(): QDialog(0){};

  // деструктор
  ~usbldialog() {
    if (fname != 0) delete fname;
    if (ptfname != 0) delete ptfname;
  }
  
public slots: 
  void browse();
  void ptbrowse();
  void ptclear();
};  

//****************************************************************
// Заголовок загрузчика
//****************************************************************
struct lhead{
  uint32_t lmode;  // режим запуска: 1 - прямой старт, 2 - через перезапуск A-core
  uint32_t size;   // размер компонента
  uint32_t adr;    // адрес загрузки компонента в память
  uint32_t offset; // смещение до компонента от начала файла
};

void usbload();