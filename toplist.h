#ifndef TOPLIST_H
#define TOPLIST_H

#include <stdint.h>

#define TOPLIST_LENGTH 3

void toplist_clear();
void toplist_add(uint16_t score);
void toplist_clear();
uint16_t toplist_read(uint8_t nth);

#endif // TOPLIST_H
