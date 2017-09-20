#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"

/**
 * erase the data storage
 */
void create_initials(void) {
    /*int i;
    for (i=0; i<INITIALS_SIZE; i++) {
        initials_data_table[i] = 0;
    }
    for (i=0; i<NUMBER_OF_GLOBALS; i++) {
        initials_table[i].type = 0;
        initials_table[i].name[0] = 0;
        initials_table[i].dim = 0;
        initials_table[i].data_len = 0;
    }*/
}

/**
 * add new symbol to table, initialise begin position in data array
 * @param symbol_name
 * @param type
 */
void add_symbol_initials(char *symbol_name, char type) {
    strcpy(initials_table[initials_idx].name, symbol_name);
    initials_table[initials_idx].type = type;
}

/**
 * find symbol in table, count position in data array
 * @param symbol_name
 * @return
 */
int find_symbol_initials(char *symbol_name) {
    int result = 0;
    for (initials_idx=0; initials_table[initials_idx].type != 0; initials_idx++) {
        if (initials_idx >= NUMBER_OF_GLOBALS) {
            error("initials table overrun");
        }
        if (astreq (symbol_name, initials_table[initials_idx].name, NAMEMAX) != 0) {
            result = 1;
            break;
        }
    }
    return result;
}

/**
 * get number of data items for given symbol
 * @param symbol_name
 * @return
 */
int get_size(char *symbol_name) {
    int result = 0;
    if (find_symbol_initials(symbol_name) != 0) {
        result = initials_table[initials_idx].dim;
    }
    return result;
}

