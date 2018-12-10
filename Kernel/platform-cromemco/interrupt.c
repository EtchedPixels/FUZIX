#include <kernel.h>
#include <irq.h>

static uint8_t irqbusy(uint8_t vec)
{
    uint16_t *p = (uint16_t *)(irqvec + vec);
    if (*p != (uint16_t)spurious)
        return 1;
    return 0;
}
    
static uint8_t irqcheck(uint8_t vec)
{
    if (irqbusy(vec & 0xFE))
        return 1;
    /* If it's odd and not 255 then check the vector above is also free */
    if ((vec & 1) && ++vec && irqbusy(vec))
        return 1;
    return 0;
}

int request_irq(uint8_t vec, void (*func)(uint8_t))
{
    /* This is fun because the Cromemco has uart 0 set to use the 8080 mode
       vectors which are odd numbers */
    if (irqcheck(vec))
        return -EBUSY;
    set_irq(vec, func);
    return 0;
}

void free_irq(uint8_t vec)
{
    if (!irqcheck(vec))
        panic("free_irq");
    set_irq(vec, spurious);
}

