#include <ui_main.h>
#include <QtWidgets>
#include "hexeditor/qhexedit.h"

class MainWindow: public QMainWindow, public Ui_MainWindow {
  
Q_OBJECT

QByteArray hexcup;
QHexEdit* hexedit;

public:
  MainWindow(QMainWindow *parent = 0);
  ~MainWindow();
public slots: 
void  SelectFwFile();  // выбор файла
void  AppendFwFile();  // добавление файла
void  SaveFwFile();    // запись полного образа на диск
void SelectPart();     // выбор раздела прошивки
void Menu_Part_Store();
void Menu_Part_Extract();  
void Menu_Part_Replace();
void Menu_Part_Delete();
void Menu_Part_MoveUp();
void Menu_Part_MoveDown();
void Terminate() { QCoreApplication::quit(); }
void regenerate_partlist();
void Menu_Part_EditHeader();  
void Disable_EditHeader();
void HeaderChanged();
void Start_Flasher();  
void Reboot_modem();  
void usbdload();
void find_ports();  
};
  