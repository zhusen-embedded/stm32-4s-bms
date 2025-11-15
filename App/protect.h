#ifndef PROTECT_H
#define PROTECT_H

#include <stdint.h>

typedef enum {
    PROT_OK = 0,
    PROT_OV,
    PROT_UV,
    PROT_OC,
    PROT_OT
} prot_state_t;

prot_state_t protect_check(const float *cells, uint8_t n, float cur, float temp);
prot_state_t protect_state(void);
uint16_t     protect_fault_code(void);
void         protect_clear(void);

#endif
