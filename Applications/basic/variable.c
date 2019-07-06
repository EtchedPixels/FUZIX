#include "basic.h"


#define VAR_TYPE	0x18
#define VAR_SIZED	0x10
#define VAR_STRING	0x08
#define VAR_ARRAY	0x04
#define VAR_DIMS	0x03
#define VAR_NAME	0xFFE0


static uint8_t *find_var(uint16_t code, uint16_t mask)
{
    while(*ptr && (*(uint16_t *)ptr & mask) != code) {
        if (ptr[1] & VAR_SIZED)
            ptr += *(uint16_t *)(ptr + 2);
        else
            ptr += 4;
    }
    if (*ptr == 0)
        return NULL;
    return ptr;
}

static uint8_t *find_number(uint16_t code)
{
    /* Match number non array */
    return find_var(code, VAR_NAME|VAR_DIMS|VAR_STRING|VAR_ARRAY);
}

static uint8_t *find_string(uint16_t code)
{
    code |= VAR_STRING;
    return find_var(code, VAR_NAME|VAR_DIMS|VAR_TYPE|VAR_ARRAY);
}

static uint8_t *find_number_array(uint16_t code)
{
    code |= VAR_ARRAY;
    /* For/next can't be an array */
    return find_var(code, VAR_NAME|VAR_DIMS|VAR_TYPE|VAR_ARRAY);
}

static uint8_t *find_string_array(uint16_t code)
{
    code |= VAR_ARRAY|VAR_STRING;
    return find_var(code, VAR_NAME|VAR_DIMS|VAR_TYPE|VAR_ARRAY);
}

static uint16_t make_code(uint8_t *p)
{
    uint16_t code;
    
    code = (*p++ & 31) << 11;
    code |= ((*p++ - 32) & 63) << 5;
    if (*p == '$') {
        p++;
        code |= VAR_STRING;
    }
    /* Not ideal if we do Sinclair style stuff due to (A TO B) syntax */
    if (*p == '(') {
        p++;
        code |= VAR_ARRAY;
    }
    return code;
}

int dim_array(uint8_t *p, uint8_t ndim, uint16_t *dims)
{
    uint16_t size;
    uint16_t *p = dims;
    uint8_t i;

    code = make_code(p);
    
    if (code & VAR_STRING)
        size = 1;
    else
        size = 2;	/* 4 for fp */
    for (i = 0; i < ndim; i++) {
        size *= *p;
        if (size < *p++)
            return -1;		/* Size overflow */
    }
    if (size + 4 < size)
        return -1;
    size += 4;

    code |= ndim;

    if (varnext + size > varend || varnext + size > varnext)
        return -2;		/* No memory */

    *(uint16_t *)varnext = code 
    varnext += 2;
    *(uint16_t *)varnext = size;
    varnext += 2;

    p = dims;
    for (i = 0; i < ndim; i++) {
        *(uint16_t *)varnext = *p++;
        varnext+=2;
    }
    memset(varnext, 0, size);
    varnext += size;
    *varnext = 0;		/* end marker */
    return 0;
}

int set_number(uint16_t code, uint16_t val)
{
    uint8_t *p = find_number(code);

    if (p == 0) {
        if (varend - varnext < 4)
            return -1;
        *(uint16_t *)varnext = code;
        *(uint16_t *)(varnext + 2) = val;
        varnext += 4;
        return 0;
    }
    if ((p[1] & VAR_TYPE) == VAR_SIZED)
        *(uint16_t *)(varnext + 4) = val;
    else
        *(uint16_t *)(varnext + 2) = val;
    return 0;
}

