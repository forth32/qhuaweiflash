#include <ui_main.h>
#include <QtWidgets>
#include "ptable.h"
#include "hexeditor/qhexedit.h"

class MainWindow: public QMainWindow, public Ui_MainWindow {
  
Q_OBJECT

QByteArray hexcup;
QHexEdit* hexedit=0;   // hex-редактор сырых образов разделов
QTableWidget* ptedit=0; // редактор таблицы разделов 
QLineEdit* oemedit=0;   // редактор oeminfo-разделов
QTableWidget* cpioedit=0; // редактор cpio-разделов

int hrow=-1;   // строка списка разделов, соответствующая текущему заголовку

enum parttypes partmode=part_bin;

public:
  MainWindow(QMainWindow *parent = 0);
  ~MainWindow(); 
void cpio_create_list(QList<cpfiledir*>*);
void cpio_delete_list();

public slots: 
void  SelectFwFile();  // выбор файла
void  AppendFwFile();  // добавление файла
void  SaveFwFile();    // запись полного образа на диск
void  OpenFwFile(QString filename); // открытие файла прошивки
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
void DataChanged();
void Start_Flasher();  
void Reboot_modem();  
void usbdload();
void find_ports();  
void EnableMenu();
void set_date();
void ShowSignInfo();
void HeadCopy();
void cpio_process_file(int row, int col);
};
  