extern int siofd; // fd для работы с Последовательным портом

void dump(void* mem,int len,long base);
int send_cmd(unsigned char* incmdbuf, int blen, unsigned char* iobuf);
int open_port();
void port_timeout();
int atcmd(char* cmd, uint8_t* rbuf);
void find_ports(QComboBox* qbox);
unsigned short crc16(uint8_t* buf, int len);
void close_port();

char* serial_port_name();
void modem_reboot();
void end_hdlc();



