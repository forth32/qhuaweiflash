// Редактор раздела nvdload 

#ifndef __NVDEDIT_H_H
#define __NVDEDIT_H_H
#include <stdint.h>
#include <QtWidgets>

// Описатели формата заголовка раздела nvdload
// Выдрано из хуавеевских исходников ядра

#define NV_FILE_MAGIC 0x766e  // nv

// Описатель каждого из компонентов 
struct nv_file_info {
    uint32_t magic;      // сигнатура 0x766e(nv)
    uint32_t off;            /*file offset in one section*/
    uint32_t len;            /*file lenght */
};

// Заголовок раздела nvdload 
// для старых чипсетов (до V7R11 включительно) поле ulSimNumEx и trash отсутствует
struct nv_dload_packet_head {

    struct nv_file_info nv_bin;      // образ nvimg
    struct nv_file_info xnv_xml;  // Основной XML-компонент
    struct nv_file_info xnv_xml2; 
    struct nv_file_info cust_xml; // Дополнительный XML-компонент
    struct nv_file_info cust_xml2; 
    struct nv_file_info xnv_map;  
    struct nv_file_info xnv_map2;  
    uint32_t ulSimNumEx;                  // Число поддерживаемых модемов минус 2
    uint8_t trash[36]; // описатели дополнительного модема, здесь не используются
//     STRU_XNV_MAP_FILE_INFO xnv_file[0]; 
};

//***********************************************************
//* Класс главного окна редактора
//***********************************************************
class nvdedit  : public QWidget {

Q_OBJECT

// Заголовок
struct nv_dload_packet_head hdr;

QVBoxLayout* vlm;
QGridLayout* lcomp;
QSpacerItem* rspacer;

QLabel* hdrlabel;
QLabel* ntype;

QLabel* comphdr1;
QLabel* comphdr2;
QLabel* comphdr3;

QLabel* name1;
QLabel* name2;
QLabel* name3;
QLabel* name4;

QLabel* size1;
QLabel* size2;
QLabel* size3;
QLabel* size4;

QPushButton* extr1;
QPushButton* extr2;
QPushButton* extr3;
QPushButton* extr4;

QPushButton* repl1;
QPushButton* repl2;
QPushButton* repl3;
QPushButton* repl4;

// номер данного разела в таблице разделов
int pnum;

// локальная копия образа раздела
uint8_t* data;
uint32_t plen;// длина-128, без хуавеевского заголовка

// тип файла
int filetype;

// копии компонентов
uint8_t* nvpart;
uint8_t* xmlpart=0;
uint8_t* custxmlpart=0;
uint8_t* xmlmap=0;

void rebuild_data();
void extractor(int type);
void replacer(int type);


public:

nvdedit(int xpnum, QWidget* parent);
~nvdedit();

public slots:

void extract1();
void extract2();
void extract3();
void extract4();
  
void replace1();
void replace2();
void replace3();
void replace4();

};



#endif 