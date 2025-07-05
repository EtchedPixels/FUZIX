#include <kernel.h>
#include <kdata.h>
#include <plt_ide.h>

#ifdef CONFIG_TD_IDE

extern void* td_io_data_reg;
extern uint8_t td_io_data_count;
extern void td_io_rblock(uint8_t *p) __naked;
extern void td_io_wblock(uint8_t *p) __naked;

void devide_read_data(uint8_t *p)
{
    td_io_data_reg = IDE_REG_DATA;
    td_io_data_count = 64;
    td_io_rblock(p);
}

void devide_write_data(uint8_t *p)
{
    uint8_t n;
    td_io_data_reg = IDE_REG_DATA;
    td_io_data_count = 64;
    td_io_wblock(p);
}

#endif
