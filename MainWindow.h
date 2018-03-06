#include <QtWidgets>
#include "ptable.h"
#include "hexeditor/qhexedit.h"

//******************************************************************************
//* Класс главного окна
//******************************************************************************
class MainWindow: public QMainWindow {
  
Q_OBJECT

QByteArray hexcup;
QHexEdit* hexedit=0;   // hex-редактор сырых образов разделов
QTableWidget* ptedit=0; // редактор таблицы разделов 
QLineEdit* oemedit=0;   // редактор oeminfo-разделов
QTableWidget* cpioedit=0; // редактор cpio-разделов
QLabel* label=0;
QShortcut* keyF3;    // обработчик F3
QShortcut* keyF11;    // обработчик F11

int hrow=-1;   // строка списка разделов, соответствующая текущему заголовку
int structure_mode_save=-1; // предыдущее состояние переключателя дамп-формат

enum parttypes partmode=part_bin;

public:
  MainWindow();
  ~MainWindow(); 
void cpio_create_list(QList<cpfiledir*>*, int);
void cpio_delete_list();

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

// Главное меню
QMenuBar *menubar;
QMenu *menu_file;
QMenu *menu_oper;
QMenu *menu_part;

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
void cpio_process_file(int row, int col);
void F11_processor();
};

// Независимые от лкасса обработчики
void head_copy();
