#ifndef __HEXEDITOR1_H
#define __HEXEDITOR1_H
#include <stdint.h>
#include <QtWidgets>

// Описатели формата заголовка раздела с ядром
// Выдрано из mkbootimg от android-sdk
#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512
#define BOOT_EXTRA_ARGS_SIZE 1024

struct boot_img_hdr
{
    uint8_t magic[BOOT_MAGIC_SIZE];

    uint32_t kernel_size;  /* size in bytes */
    uint32_t kernel_addr;  /* physical load addr */

    uint32_t ramdisk_size; /* size in bytes */
    uint32_t ramdisk_addr; /* physical load addr */

    uint32_t second_size;  /* size in bytes */
    uint32_t second_addr;  /* physical load addr */

    uint32_t tags_addr;    /* physical addr for kernel tags */
    uint32_t page_size;    /* flash page size we assume */
    uint32_t dt_size;      /* device tree in bytes */
    uint32_t unused;       /* future expansion: should be 0 */

    uint8_t name[BOOT_NAME_SIZE]; /* asciiz product name */

    uint8_t cmdline[BOOT_ARGS_SIZE];

    uint32_t id[8]; /* timestamp / checksum / sha1 / etc */

    /* Supplemental command line data; kept here to maintain
     * binary compatibility with older versions of mkbootimg */
    uint8_t extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
} __attribute__((packed));

//***********************************************************
//* Класс главного окна редактора
//***********************************************************
class kerneledit  : public QWidget {

Q_OBJECT

// указатель на структуру заголовка
struct boot_img_hdr* hdr;
// локальная копия образа раздела
uint8_t* pdata;
uint32_t plen;

QVBoxLayout* vlm;
QFormLayout* flm;
QGridLayout* lcomp;
QSpacerItem* rspacer;

QLabel* hdrlabel;
QLabel* parmlabel;
QLabel* pgslabel;
QLabel* tagslabel;
QLabel* dtlabel;
QLabel* pname;
QLineEdit* cmdline;

QLabel* complabel;
QLabel* kcomp;
QLabel* r1comp;
QLabel* r2comp;

QPushButton* kext;
QPushButton* krepl;
QPushButton* r1ext;
QPushButton* r1repl;
QPushButton* r2ext;
QPushButton* r2repl;

void setup_adr(int type, uint32_t* adr, uint32_t* len, QString* filename);
void extractor(int type);
void replacer(int type);

public:

kerneledit(uint8_t* data, uint32_t len, QWidget* parent);
~kerneledit();

public slots:

void kextract();
void r1extract();
void r2extract();

void kreplace();
void r1replace();
void r2replace();

};


/*
** +-----------------+ 
** | boot header     | 1 page
** +-----------------+
** | kernel          | n pages  
** +-----------------+
** | ramdisk         | m pages  
** +-----------------+
** | second stage    | o pages
** +-----------------+
** | device tree     | p pages
** +-----------------+
**
** n = (kernel_size + page_size - 1) / page_size
** m = (ramdisk_size + page_size - 1) / page_size
** o = (second_size + page_size - 1) / page_size
** p = (dt_size + page_size - 1) / page_size
**
** 0. all entities are page_size aligned in flash
** 1. kernel and ramdisk are required (size != 0)
** 2. second is optional (second_size == 0 -> no second)
** 3. load each element (kernel, ramdisk, second) at
**    the specified physical address (kernel_addr, etc)
** 4. prepare tags at tag_addr.  kernel_args[] is
**    appended to the kernel commandline in the tags.
** 5. r0 = 0, r1 = MACHINE_TYPE, r2 = tags_addr
** 6. if second_size != 0: jump to second_addr
**    else: jump to kernel_addr
*/


#endif // #define __HEXEDITOR_H