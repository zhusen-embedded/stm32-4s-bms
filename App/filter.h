#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

#define MED_N 5

void med_push(uint16_t v);
uint16_t med_get(void);

#endif
