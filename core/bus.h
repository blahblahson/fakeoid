#ifndef CORE_BUS_H_
#define CORE_BUS_H_

#include <stddef.h> /* FIXME: issues with flycheck and NULL being undefined */

#include "../utils/list.h"

typedef enum
{
    PIN_STATE_NONE, /* "high impedance state" */
    PIN_STATE_HI,
    PIN_STATE_LO,
    PIN_STATE_INVALID,
} pin_state_t;

typedef enum
{
    PIN_TYPE_INPUT,
    PIN_TYPE_OUTPUT,
    PIN_TYPE_BIDIRECTIONAL,
} pin_type_t;

typedef struct
{
    const pin_type_t type;
    pin_state_t state;
    struct dl_list list;
} pin_t;

void bus_init(void);
pin_state_t pin_evaluate(pin_t *pin);
void pin_set(pin_t *pin, pin_state_t state);

/* FIXME: Move this to cpu and ram modules */

/* CPU */
extern pin_t cpu_addr_bus[16];
extern pin_t cpu_data_bus[8];
extern pin_t cpu_rwb;

/* RAM */
extern pin_t ram_addr_bus[15];
extern pin_t ram_data_bus[8];
extern pin_t ram_we;
extern pin_t ram_oe;
extern pin_t ram_cs;
// oe and cs - ignore for now

#endif /* CORE_BUS_H_ */
