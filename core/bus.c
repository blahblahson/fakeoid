#include <assert.h>
#include <stdbool.h>

#include "bus.h"

/* CPU */
pin_t cpu_addr_bus[16] = { { PIN_TYPE_OUTPUT, PIN_STATE_NONE, { NULL, NULL } } };
pin_t cpu_data_bus[8] = { { PIN_TYPE_BIDIRECTIONAL, PIN_STATE_NONE, { NULL, NULL } } };
pin_t cpu_rwb = { PIN_TYPE_OUTPUT, PIN_STATE_NONE, { NULL, NULL } };

/* RAM */
pin_t ram_addr_bus[15] = { { PIN_TYPE_INPUT, PIN_STATE_NONE, { NULL, NULL } } };
pin_t ram_data_bus[8] = { { PIN_TYPE_BIDIRECTIONAL, PIN_STATE_NONE, { NULL, NULL } } };
pin_t ram_we = { PIN_TYPE_INPUT, PIN_STATE_NONE, { NULL, NULL } };
pin_t ram_oe = { PIN_TYPE_INPUT, PIN_STATE_NONE, { NULL, NULL } };
pin_t ram_cs = { PIN_TYPE_INPUT, PIN_STATE_NONE, { NULL, NULL } };
// oe and cs - ignore for now

/*
 * Get around the fact that the default initialization above doesn't work well
 * for the list element
 */
static void init_pin(pin_t *pin)
{
    if (pin->list.next == NULL && pin->list.prev == NULL)
        pin->list = (struct dl_list)DL_LIST_HEAD_INIT(pin->list);
}

static void init_pins(void)
{
    for (int i = 0; i < 16; i++)
        init_pin(&cpu_addr_bus[i]);

    for (int i = 0; i < 15; i++)
        init_pin(&ram_addr_bus[i]);

    for (int i = 0; i < 8; i++)
    {
        init_pin(&cpu_data_bus[i]);
        init_pin(&ram_data_bus[i]);
    }

    init_pin(&cpu_rwb);
    init_pin(&ram_we);
    init_pin(&ram_oe);
    init_pin(&ram_cs);
}

static void init_cpu_ram_bus(void)
{
    for (int i = 0; i < 15; i++)
        dl_list_add(&cpu_addr_bus[i].list, &ram_addr_bus[i].list);

    for (int i = 0; i < 8; i++)
        dl_list_add(&cpu_data_bus[i].list, &ram_data_bus[i].list);

    dl_list_add(&cpu_rwb.list, &ram_we.list);
    dl_list_add(&cpu_addr_bus[14].list, &ram_oe.list);
}

static bool pin_asserted(const pin_t *pin)
{
    if (pin->type != PIN_TYPE_OUTPUT && pin->type != PIN_TYPE_BIDIRECTIONAL)
        return false;

    if (pin->state == PIN_STATE_NONE)
        return false;

    /*
     * FIXME: What about PIN_STATE_INVALID? It shouldn't be possible
     * to set I guess
     */
    return true;
}

void pin_set(pin_t *pin, pin_state_t state)
{
    assert(state != PIN_STATE_INVALID);
    assert(pin->type != PIN_TYPE_INPUT);
    pin->state = state;
}

pin_state_t pin_evaluate(pin_t *pin)
{
    pin_t *p;
    pin_state_t state = PIN_STATE_NONE;

    dl_list_for_each(p, &pin->list, pin_t, list)
    {
        if (!pin_asserted(p))
            continue;

        /*
         * If there is more than one asserted pin on the bus, it's an error, and
         * we indicate this with an invalid state.
         */
        if (state != PIN_STATE_NONE)
        {
            state = PIN_STATE_INVALID;
            break;
        }

        state = p->state;
    }

    return state;
}

void bus_init(void)
{
        init_pins();
        init_cpu_ram_bus();
}
