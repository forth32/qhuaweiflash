//**********************************************************************
//* Формирование интерфейса главного окна
//*********************************************************************

#include "MainWindow.h"

//************************************************ 
//* Конструктор класса главного окна
//************************************************
void   MainWindow::MainWindow() : QMainWindow(0) {

// MainWindow->resize(920, 737);

// Формируем иконку главного окна
icon.addFile(QStringLiteral(":/icon.ico"), QSize(), QIcon::Normal, QIcon::Off);
setWindowIcon(icon);

// Сплиттер - корневой виджет окна
centralwidget=new QSplitter(Qt::Horizontal, this);
setCentralWidget(centralwidget);

// Левое окно - редактор заголовков
hdrpanel=new QWidget(centralwidget);
centralwidget->addWidget(hdrpanel);
vlhdr=new QVboxLayout(hdrpanel);

// Список разделов
hdlbl1=new QLabel("Список разделов",hdrpanel);
vlhdr->addWidget(hdlbl1);
partlist = new QListWidget(hdrpanel);
vlhdr->addWidget(partlist);

hdlbl2=new QLabel("Заголовок раздела",hdrpanel);
vlhdr->addWidget(hdlbl2);

// Компоновщик для параметров редактирования
lphdr=new QFormLayout(0);
vlhdr->addLayout(lphdr);
// элементы редактора заголовка
pcode = new QLineEdit(hdrpanel);
pcode->setReadOnly(true);
lphdr->addRow("Код раздела",pcode);

Platform_input = new QLineEdit(hdrpanel);
Platform_input->setReadOnly(true);
lphdr->addRow("Платформа",Platform_input);

Date_input = new QLineEdit(hdrpanel);
Date_input->setReadOnly(true);
lphdr->addRow("Дата",Date_input);

Time_input = new QLineEdit(hdrpanel);
Time_input->setReadOnly(true);
lphdr->addRow("Время",Time_input);

setdate = new QToolButton(hdrpanel);
setdate->setText("Текущая дата");
vlhdr->addWidget(setdate);

hdlbl3=new QLabel("Вресия прошивки",hdrpanel);
vlhdr->addWidget(hdlbl3);

Version_input = new QLineEdit(hdrpanel);
Version_input->setReadOnly(true);
vlhdr->addWidget(Version_input);

// Правое окно - редактор разделов
edpanel=new QWidget(centralwidget);
centralwidget->addWidget(edpanel);
EditorLayout=new QVBoxLayout(edpanel);

// Кнопки сырой-форматный
laymode=new QHBoxLayout(0);
modebuttons = new QGroupBox("Вид");

dump_mode = new QRadioButton("НЕХ-дамп");
laymode->addWidget(dump_mode);

structure_mode = new QRadioButton("Форматный");
structure_mode->setChecked(true);
laymode->addWidget(structure_mode);

modebuttons->setLayout(laymode);
EditorLayout->addWidget(modebuttons);

// Статусбар
statusbar = new QStatusBar(this);
setStatusBar(statusbar);

// Выбор порта
plbl=new QLabel("Порт модема:");
statusbar->addPermanentWidget(plbl);

PortSelector = new QComboBox(groupBox);
statusbar->addPermanentWidget(PortSelector);

RefreshPorts = new QToolButton(groupBox);
RefreshPorts->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
statusbar->addPermanentWidget(RefreshPorts);

// Обработчики главного меню
fileopen = new QAction(MainWindow);
fileopen->setObjectName(QStringLiteral("fileopen"));
fileappend = new QAction(MainWindow);
fileappend->setObjectName(QStringLiteral("fileappend"));
fileappend->setEnabled(false);
part_store = new QAction(MainWindow);
part_store->setObjectName(QStringLiteral("part_store"));
part_extract = new QAction(MainWindow);
part_extract->setObjectName(QStringLiteral("part_extract"));
part_replace = new QAction(MainWindow);
part_replace->setObjectName(QStringLiteral("part_replace"));
file_exit = new QAction(MainWindow);
file_exit->setObjectName(QStringLiteral("file_exit"));
filesave = new QAction(MainWindow);
filesave->setObjectName(QStringLiteral("filesave"));
filesave->setEnabled(false);
MoveUp = new QAction(MainWindow);
MoveUp->setObjectName(QStringLiteral("MoveUp"));
MoveDown = new QAction(MainWindow);
MoveDown->setObjectName(QStringLiteral("MoveDown"));
Delete = new QAction(MainWindow);
Delete->setObjectName(QStringLiteral("Delete"));
part_copy_header = new QAction(MainWindow);
part_copy_header->setObjectName(QStringLiteral("part_copy_header"));
Menu_Oper_flash = new QAction(MainWindow);
Menu_Oper_flash->setObjectName(QStringLiteral("Menu_Oper_flash"));
Menu_Oper_flash->setEnabled(false);
Menu_Oper_USBDload = new QAction(MainWindow);
Menu_Oper_USBDload->setObjectName(QStringLiteral("Menu_Oper_USBDload"));
Menu_Oper_Reboot = new QAction(MainWindow);
Menu_Oper_Reboot->setObjectName(QStringLiteral("Menu_Oper_Reboot"));
Menu_Oper_signinfo = new QAction(MainWindow);
Menu_Oper_signinfo->setObjectName(QStringLiteral("Menu_Oper_signinfo"));
Menu_Oper_signinfo->setEnabled(false);

// Главное меню
menubar = new QMenuBar(MainWindow);
// menubar->setObjectName(QStringLiteral("menubar"));
// menubar->setGeometry(QRect(0, 0, 920, 30));
menu_file = new QMenu(menubar);
menu_file->setObjectName(QStringLiteral("menu_file"));
menu_oper = new QMenu(menubar);
menu_oper->setObjectName(QStringLiteral("menu_oper"));
menu_oper->setEnabled(true);
menu_part = new QMenu(menubar);
menu_part->setObjectName(QStringLiteral("menu_part"));
menu_part->setEnabled(true);
setMenuBar(menubar);

// Элементы главного меню
menubar->addAction(menu_file->menuAction());
menubar->addAction(menu_part->menuAction());
menubar->addAction(menu_oper->menuAction());
menu_file->addAction(fileopen);
menu_file->addAction(fileappend);
menu_file->addSeparator();
menu_file->addAction(filesave);
menu_file->addSeparator();
menu_file->addAction(file_exit);
menu_oper->addAction(Menu_Oper_flash);
menu_oper->addAction(Menu_Oper_USBDload);
menu_oper->addAction(Menu_Oper_Reboot);
menu_oper->addSeparator();
menu_oper->addAction(Menu_Oper_signinfo);
menu_part->addAction(MoveUp);
menu_part->addAction(MoveDown);
menu_part->addAction(Delete);
menu_part->addSeparator();
menu_part->addAction(part_copy_header);
menu_part->addSeparator();
menu_part->addAction(part_store);
menu_part->addAction(part_extract);
menu_part->addAction(part_replace);

// Установка обработчиков сигналов
QObject::connect(fileopen, SIGNAL(triggered()), MainWindow, SLOT(SelectFwFile()));
QObject::connect(partlist, SIGNAL(itemActivated(QListWidgetItem*)), MainWindow, SLOT(SelectPart()));
QObject::connect(fileappend, SIGNAL(triggered()), MainWindow, SLOT(AppendFwFile()));
QObject::connect(part_extract, SIGNAL(triggered()), MainWindow, SLOT(Menu_Part_Extract()));
QObject::connect(part_store, SIGNAL(triggered()), MainWindow, SLOT(Menu_Part_Store()));
QObject::connect(partlist, SIGNAL(itemClicked(QListWidgetItem*)), MainWindow, SLOT(SelectPart()));
QObject::connect(part_replace, SIGNAL(triggered()), MainWindow, SLOT(Menu_Part_Replace()));
QObject::connect(file_exit, SIGNAL(triggered()), MainWindow, SLOT(Terminate()));
QObject::connect(filesave, SIGNAL(triggered()), MainWindow, SLOT(SaveFwFile()));
QObject::connect(Delete, SIGNAL(triggered()), MainWindow, SLOT(Menu_Part_Delete()));
QObject::connect(MoveUp, SIGNAL(triggered()), MainWindow, SLOT(Menu_Part_MoveUp()));
QObject::connect(MoveDown, SIGNAL(triggered()), MainWindow, SLOT(Menu_Part_MoveDown()));
QObject::connect(partlist, SIGNAL(currentRowChanged(int)), MainWindow, SLOT(Disable_EditHeader()));
QObject::connect(Menu_Oper_flash, SIGNAL(triggered()), MainWindow, SLOT(Start_Flasher()));
QObject::connect(Menu_Oper_Reboot, SIGNAL(triggered()), MainWindow, SLOT(Reboot_modem()));
QObject::connect(Menu_Oper_USBDload, SIGNAL(triggered()), MainWindow, SLOT(usbdload()));
QObject::connect(setdate, SIGNAL(clicked()), MainWindow, SLOT(set_date()));
QObject::connect(Menu_Oper_signinfo, SIGNAL(triggered()), MainWindow, SLOT(ShowSignInfo()));
QObject::connect(dump_mode, SIGNAL(toggled(bool)), MainWindow, SLOT(SelectPart()));
QObject::connect(partlist, SIGNAL(currentRowChanged(int)), MainWindow, SLOT(SelectPart()));
QObject::connect(part_copy_header, SIGNAL(triggered()), MainWindow, SLOT(HeadCopy()));
QObject::connect(RefreshPorts, SIGNAL(clicked()), MainWindow, SLOT(find_ports()));

QMetaObject::connectSlotsByName(MainWindow);
  
// внешняя ссылка на выбиралку порта
pselector=PortSelector;

// создание класса таблицы разделов
ptable=new ptable_list;

// заполнение списка портов
find_ports();

// обнуление указателей на редакторы разделов
hexedit=0;
ptedit=0;
cpioedit=0;

// открываем файл из командной строки
if (fwfilename != 0) {
  OpenFwFile(*fwfilename);
}


//*****************************************
//* Деструктор класса
//*****************************************
MainWindow::~MainWindow() {

delete ptable;  
}
