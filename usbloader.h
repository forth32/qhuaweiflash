#include <QtWidgets>
#include "ui_usbloader.h"

class usbloader: public QDialog, public Ui_usbloader {

Q_OBJECT

void leave();

// хранилище каталога компонентов загрузчика
struct {
  uint32_t lmode;  // режим запуска: 1 - прямой старт, 2 - через перезапуск A-core
  uint32_t size;   // размер компонента
  uint32_t adr;    // адрес загрузки компонента в память
  uint32_t offset; // смещение до компонента от начала файла
} part[5];

// массив буферов для загрузки компонентов
uint8_t* pbuf[5]={0,0,0,0,0};

uint16_t numparts; // число компонентов для загрузки
void fastboot_only();

public:
      usbloader(QWidget *parent = 0);
public slots: 
       int exec();
       void browse();
       void ptbrowse();
};

