//**********************************************************************
//* Формирование интерфейса главного окна
//*********************************************************************

#include "MainWindow.h"
// ссылка на селектор портов
extern QComboBox* pselector;
extern QString* fwfilename;

//************************************************ 
//* Конструктор класса главного окна
//************************************************
MainWindow::MainWindow(): QMainWindow() {

this->resize(1000, 737);

// Шрифт для надписей на основной панели
QFont font;
font.setPointSize(14);
font.setBold(true);
font.setWeight(75);

// Формируем иконку главного окна
icon.addFile(QStringLiteral(":/icon.ico"), QSize(), QIcon::Normal, QIcon::Off);
setWindowIcon(icon);

// Сплиттер - корневой виджет окна
centralwidget=new QSplitter(Qt::Horizontal, this);
setCentralWidget(centralwidget);

// Левое окно - редактор заголовков
hdrpanel=new QWidget(centralwidget);
centralwidget->addWidget(hdrpanel);
centralwidget->setStretchFactor(0,1);
vlhdr=new QVBoxLayout(hdrpanel);

// Список разделов
hdlbl1=new QLabel("Список разделов",hdrpanel);
hdlbl1->setFont(font);
vlhdr->addWidget(hdlbl1);

partlist = new QListWidget(hdrpanel);
vlhdr->addWidget(partlist);

hdlbl2=new QLabel("Заголовок раздела",hdrpanel);
hdlbl2->setFont(font);
vlhdr->addWidget(hdlbl2);

// Компоновщик для параметров редактирования
lphdr=new QFormLayout(0);
vlhdr->addLayout(lphdr);
// элементы редактора заголовка
pcode = new QLineEdit(hdrpanel);
pcode->setMaxLength(4);
pcode->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
pcode->setReadOnly(true);
lphdr->addRow("Код раздела",pcode);

Platform_input = new QLineEdit(hdrpanel);
Platform_input->setMaxLength(8);
Platform_input->setReadOnly(true);
Platform_input->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
lphdr->addRow("Платформа",Platform_input);

Date_input = new QLineEdit(hdrpanel);
Date_input->setMaxLength(16);
Date_input->setReadOnly(true);
Date_input->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
lphdr->addRow("Дата",Date_input);

Time_input = new QLineEdit(hdrpanel);
Time_input->setMaxLength(16);
Time_input->setReadOnly(true);
Time_input->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
lphdr->addRow("Время",Time_input);

setdate = new QToolButton(hdrpanel);
setdate->setText("Установить текущую дату");
vlhdr->addWidget(setdate);

hdlbl3=new QLabel("Версия прошивки",hdrpanel);
vlhdr->addWidget(hdlbl3);

QSize qs=Time_input->sizeHint();
qDebug()  <<qs;
qs.rwidth() *=2;
qDebug()  <<qs;
Version_input = new QLineEdit(hdrpanel);
Version_input->setMaxLength(32);
Version_input->setReadOnly(true);
Version_input->resize(qs);
Version_input->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
vlhdr->addWidget(Version_input);

// Правое окно - редактор разделов
edpanel=new QWidget(centralwidget);
centralwidget->addWidget(edpanel);
centralwidget->setStretchFactor(1,5);
EditorLayout=new QVBoxLayout(edpanel);

// Кнопки сырой-форматный
laymode=new QHBoxLayout(0);
modebuttons = new QGroupBox("Вид");
// modebuttons->setFrameStyle(QFrame::Panel | QFrame::Sunken);
modebuttons->setFlat(false);

dump_mode = new QRadioButton("НЕХ-дамп");
laymode->addWidget(dump_mode);

structure_mode = new QRadioButton("Форматный");
structure_mode->setChecked(true);
laymode->addWidget(structure_mode);

modebuttons->setLayout(laymode);
modebuttons->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
EditorLayout->addWidget(modebuttons,0);

// Статусбар
statusbar = new QStatusBar(this);
setStatusBar(statusbar);

// Выбор порта
plbl=new QLabel("Порт модема:");
statusbar->addPermanentWidget(plbl);

PortSelector = new QComboBox(centralwidget);
statusbar->addPermanentWidget(PortSelector);

RefreshPorts = new QToolButton(centralwidget);
RefreshPorts->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
statusbar->addPermanentWidget(RefreshPorts);

// Главное меню
menubar = new QMenuBar(this);
menu_file = new QMenu("Файл",menubar);
menubar->addAction(menu_file->menuAction());

menu_part = new QMenu("Раздел",menubar);
menu_part->setEnabled(true);
menubar->addAction(menu_part->menuAction());

menu_oper = new QMenu("Операции",menubar);
menu_oper->setEnabled(true);
menubar->addAction(menu_oper->menuAction());

setMenuBar(menubar);

// Обработчики главного меню
fileopen = new QAction("Открыть",this);
fileopen->setShortcut(QKeySequence::Open);
menu_file->addAction(fileopen);

fileappend = new QAction("Добавить",this);
fileappend->setEnabled(false);
menu_file->addAction(fileappend);
menu_file->addSeparator();

filesave = new QAction("Сохранить",this);
filesave->setEnabled(false);
filesave->setShortcut(QKeySequence::Save);
menu_file->addAction(filesave);
menu_file->addSeparator();

file_exit = new QAction("Выход",this);
file_exit->setShortcut(QKeySequence::Quit);
menu_file->addAction(file_exit);
//----------------
part_store = new QAction("Извлечь без заголовка",this);
menu_part->addAction(part_store);

part_extract = new QAction("Извлечь с заголовком",this);
menu_part->addAction(part_extract);

part_replace = new QAction("Заменить образ раздела",this);
menu_part->addAction(part_replace);

MoveUp = new QAction("Переместить вверх",this);
MoveUp->setShortcut(QKeySequence("Ctrl+Up"));
menu_part->addAction(MoveUp);

MoveDown = new QAction("Переместить вниз",this);
MoveDown->setShortcut(QKeySequence("Ctrl+Down"));
menu_part->addAction(MoveDown);

Delete = new QAction("Удалить",this);
Delete->setShortcut(QKeySequence("Ctrl+Del"));
menu_part->addAction(Delete);

part_copy_header = new QAction("Копировать заголовок",this);
menu_part->addAction(part_copy_header);
//----------------
Menu_Oper_flash = new QAction("Прошть модем",this);
Menu_Oper_flash->setEnabled(false);
Menu_Oper_flash->setShortcut(QKeySequence("Alt+B"));
menu_oper->addAction(Menu_Oper_flash);

Menu_Oper_USBDload = new QAction("Загрузка usbdloader",this);
Menu_Oper_USBDload->setShortcut(QKeySequence("Alt+U"));
menu_oper->addAction(Menu_Oper_USBDload);
menu_oper->addSeparator();

Menu_Oper_Reboot = new QAction("Перезагрузка модема",this);
menu_oper->addAction(Menu_Oper_Reboot);
menu_oper->addSeparator();

Menu_Oper_signinfo = new QAction("Информация о цифровой подписи",this);
Menu_Oper_signinfo->setShortcut(QKeySequence("Alt+D"));
Menu_Oper_signinfo->setEnabled(false);
menu_oper->addAction(Menu_Oper_signinfo);

// Установка обработчиков сигналов
connect(fileopen, SIGNAL(triggered()), this, SLOT(SelectFwFile()));
connect(partlist, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(SelectPart()));
connect(fileappend, SIGNAL(triggered()), this, SLOT(AppendFwFile()));
connect(part_extract, SIGNAL(triggered()), this, SLOT(Menu_Part_Extract()));
connect(part_store, SIGNAL(triggered()), this, SLOT(Menu_Part_Store()));
connect(partlist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(SelectPart()));
connect(part_replace, SIGNAL(triggered()), this, SLOT(Menu_Part_Replace()));
connect(file_exit, SIGNAL(triggered()), this, SLOT(Terminate()));
connect(filesave, SIGNAL(triggered()), this, SLOT(SaveFwFile()));
connect(Delete, SIGNAL(triggered()), this, SLOT(Menu_Part_Delete()));
connect(MoveUp, SIGNAL(triggered()), this, SLOT(Menu_Part_MoveUp()));
connect(MoveDown, SIGNAL(triggered()), this, SLOT(Menu_Part_MoveDown()));
connect(partlist, SIGNAL(currentRowChanged(int)), this, SLOT(Disable_EditHeader()));
connect(Menu_Oper_flash, SIGNAL(triggered()), this, SLOT(Start_Flasher()));
connect(Menu_Oper_Reboot, SIGNAL(triggered()), this, SLOT(Reboot_modem()));
connect(Menu_Oper_USBDload, SIGNAL(triggered()), this, SLOT(usbdload()));
connect(setdate, SIGNAL(clicked()), this, SLOT(set_date()));
connect(Menu_Oper_signinfo, SIGNAL(triggered()), this, SLOT(ShowSignInfo()));
connect(dump_mode, SIGNAL(toggled(bool)), this, SLOT(SelectPart()));
connect(partlist, SIGNAL(currentRowChanged(int)), this, SLOT(SelectPart()));
connect(part_copy_header, SIGNAL(triggered()), this, SLOT(HeadCopy()));
connect(RefreshPorts, SIGNAL(clicked()), this, SLOT(find_ports()));

// QMetaObject::connectSlotsByName(this);
  
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
}

//*****************************************
//* Деструктор класса
//*****************************************
MainWindow::~MainWindow() {

delete ptable;  
}
