//-------------- Редактор двоичных образов NVRAM ----------------------------
#ifndef _NVEXPLORER_H_
#define _NVEXPLORER_H_
#include <stdint.h>
#include <QtWidgets>




//------------------------- Структуры данных файла nvram ------------------------------
// Хуавеевские типы данных
#define U32 uint32_t 
#define U16 uint16_t
#define U8 uint8_t

#define FILE_MAGIC_NUM 0x224e4944 // Сигнатура заголовка файла

//  Структура файла NVRAM
// 
//------- управляющая Структура -----------------
// +00 Заголовок (nvfile_header) - 96 байта
// +file_offset - каталог файлов
// +item_offset - каталог ячеек
// 4 байта CRC управляющей структуры
//-------- Данные ячеек ----------------------
// данные идут последовательно ячейка за ячейкой, без разрывов
//


// Струкура заголовка nv-файла
struct nvfile_header {

    U32 magicnum;   // сигнатура
    U32 ctrl_size;  // размер управляющих структур (смещение до данных)
    U16 version;    // * file version * /
    U8 modem_num;   // номер модема для мультимодемных конфигураций
    U8 crcflag;     // признак наличия CRC
    U32 file_offset; // смещение до списка файлов
    U32 file_num;    // число файлов в списке 
    U32 file_size;   // размер списка файлов
    U32 item_offset;  // смещение до списка ячеек
    U32 item_count;   // число ячеек в списке
    U32 item_size;    // размер списка ячеек
    U8 reserve2 [12];
    U32 timetag [4]; // отметка воемени
    U8 product_version [32]; // версия устройства
};

//  Элемент каталога файлов
struct nv_file {
    U32 id; // номер файла
    U8 name [28]; // имя файла
    U32 size; // размер файла
    U32 offset; // смещение до файла
};

// Элемент каталога ячеек
struct nv_item {
    U16 id; // номер ячейки
    U16 len; // размер в байтах
    U32 off; // смещение от начала файла
    U16 file_id; // файл, к которому относится ячейка
    U16 priority; // приоритет ячейки
    U8 modem_num; // номер модема
    U8 reserved [3]; 
};


//***********************************************************
//* Класс главного окна редактора
//***********************************************************
class nvexplorer  : public QMainWindow {

Q_OBJECT

bool changed=false;
uint8_t* pdata;
uint32_t plen;

struct nvfile_header nvhd; // заголовок nvram-файла
int crcmode;
// Смещение до поля CRC в образе nvram
uint32_t crcoff;
// Каталог файлов
struct nv_file flist[15];
// каталог ячеек
struct nv_item* itemlist;

// Подпрограммы библиотеки nvio для доступа к структурам nvram
uint32_t fileoff(int fid);
int32_t fileidx(int fid);
uint32_t itemoff_idx(int idx);
int32_t itemidx(int item);
int32_t itemoff (int item);
int32_t itemlen (int item);
int load_item(int item, char* buf);



QWidget* central;
QSettings* config;
QVBoxLayout* vlm;

// таблица nvram
QTableWidget* nvtable;

// Главное меню
QMenuBar* menubar;
QMenu* menu_file;
QMenu* menu_edit;
QMenu* menu_view;

// тулбар
QToolBar* toolbar;

public:
nvexplorer(uint8_t* data, uint32_t len);
~nvexplorer();

public slots:
// void save_all();  

};

char* find_desc(int item);

#endif // _NVEXPLORER_H_