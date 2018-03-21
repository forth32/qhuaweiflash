#include <QtWidgets>
#include "ptable.h"
#include "hexeditor.h"
#include "kerneledit.h"
#include "nvdedit.h"
#include "cpio.h"

//******************************************************************************
//* Класс главного окна
//******************************************************************************
class MainWindow: public QMainWindow {
  
Q_OBJECT

QTableWidget* ptedit=0; // редактор таблицы разделов 
QLineEdit* oemedit=0;   // редактор oeminfo-разделов
QLabel* label=0;
QSpacerItem* spacer=0; // подпорка под короткие формы редакторов
hexeditor* hexedit=0;
kerneledit* kedit=0;  // редактор разделов kernel
nvdedit* nvedit=0;  // редактор разделов kernel
cpioedit* cpio=0;   // редактор файловых разделов

// Хранилище настроек
QSettings* config;

int hrow=-1;   // строка списка разделов, соответствующая текущему заголовку
int structure_mode_save=-1; // предыдущее состояние переключателя дамп-формат

enum parttypes partmode=part_bin;

public:

MainWindow();
virtual ~MainWindow(); 

// Базовый виджет - вертикальный сплиттер
QSplitter *centralwidget;

// Иконка главного окна
QIcon icon;

// Обработчики меню
QAction *fileopen;
QAction *fileappend;
QAction *part_store;
QAction *part_extract;
QAction *part_replace;
QAction *file_exit;
QAction *filesave;
QAction *MoveUp;
QAction *MoveDown;
QAction *Delete;
QAction *part_copy_header;
QAction *Menu_Oper_flash;
QAction *Menu_Oper_USBDload;
QAction *Menu_Oper_Reboot;
QAction *Menu_Oper_signinfo;

// Элементы интерфейса

// Элементы редактора заголовка
QWidget* hdrpanel; // корневой виджет
QVBoxLayout* vlhdr; // основной вертикальный компоновщик
QLabel* hdlbl1;
QListWidget *partlist; // список разделов
QLabel* hdlbl2;
QFormLayout* lphdr;    // редакторы полей заголовка
QLineEdit *Date_input;
QLineEdit *Time_input;
QToolButton *setdate;
QLineEdit *Version_input;
QLineEdit *pcode;
QLabel* hdlbl3;
QLineEdit *Platform_input;

// Элементы редактора раздела
QWidget* edpanel;
QVBoxLayout* EditorLayout;

// Кнопки сырой-форматный
QBoxLayout* laymode;
QGroupBox *modebuttons;
QRadioButton *dump_mode;
QRadioButton *structure_mode;

// линия-разделитель
QFrame* hframe;

// Главное меню
QMenuBar *menubar;
QMenu *menu_file;
QMenu *menu_oper;
QMenu *menu_part;
QMenu* menu_edit;

// Статусбар
QStatusBar *statusbar;
// Выбор порта
QLabel* plbl;
QComboBox *PortSelector;
QToolButton *RefreshPorts;

// Слоты обработчиков главного меню
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
};

// Независимые от лкасса обработчики
void head_copy();
extern MainWindow* mw;
