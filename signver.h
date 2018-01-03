int32_t send_signver();
int32_t search_sign();

// Параметры текущей цифровой подписи

extern int32_t signlen;  // длина подписи
// Хеш открытого ключа для ^signver
extern char signver_hash[100];


