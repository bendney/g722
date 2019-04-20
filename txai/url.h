#ifndef URL_H
#define URL_H
#include <stdint.h>

int php_url_decode(char *str, int len);
char *php_url_encode(char const *s, int len, int *new_length);

char *base64_encode(const unsigned char *data, int input_length,
		int * output_length);
unsigned char *base64_decode(const char *data, int input_length,
		int * output_length);

#endif /* URL_H */
