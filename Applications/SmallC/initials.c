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
    initials_data_idx = 0;
    for (initials_idx=0; initials_table[initials_idx].type != 0; initials_idx++) {
        if (initials_idx >= NUMBER_OF_GLOBALS) {
            error("initials table overrun");
        }
        if (astreq (symbol_name, initials_table[initials_idx].name, NAMEMAX) != 0) {
            result = 1;
            break;
        } else { // move to next symbol
            // count position in data array
            initials_data_idx += initials_table[initials_idx].data_len;
        }
    }
    return result;
}

/**
 * add data to table for given symbol
 * @param symbol_name
 * @param type
 * @param value
 * @param tag
 */
void add_data_initials(char *symbol_name, int type, int value, TAG_SYMBOL *tag) {
    int position;
    if (find_symbol_initials(symbol_name) == 0) {
        add_symbol_initials(symbol_name, tag == 0 ? type : STRUCT);
    }
    if (tag != 0) {
        // find number of members, dim is total number of values added
        int index = initials_table[initials_idx].dim % tag->number_of_members;
        int member_type = member_table[tag->member_idx + index].type;
        // add it recursively
        add_data_initials(symbol_name, member_type, value, 0);
    } else {
        position = initials_table[initials_idx].data_len;
        if (type & CCHAR) {
            initials_data_table[initials_data_idx + position] = 0xff & value;
            initials_table[initials_idx].data_len += 1;
        } else if (type & CINT) {
            initials_data_table[initials_data_idx + position] = (0xff00 & value) >> 8;
            initials_data_table[initials_data_idx + position + 1] = 0xff & value;
            initials_table[initials_idx].data_len += INTSIZE;
        }
        initials_table[initials_idx].dim += 1;
    }
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

/**
 * get item at position
 * @param symbol_name
 * @param position
 * @param itag index of tag in tag table
 * @return
 */
int get_item_at(char *symbol_name, int position, TAG_SYMBOL *tag) {
    int result = 0, i, type;
    if (find_symbol_initials(symbol_name) != 0) {
        if (initials_table[initials_idx].type & CCHAR) {
            result = initials_data_table[initials_data_idx + position];
        } else if (initials_table[initials_idx].type & CINT) {
            position *= INTSIZE;
            result = (initials_data_table[initials_data_idx + position] << 8) +
                    (unsigned char)initials_data_table[initials_data_idx + position+1];
        } else if (initials_table[initials_idx].type == STRUCT) {
            // find number of members
            int number_of_members = tag->number_of_members;
            // point behind the last full struct
            int index = (position / number_of_members) * tag->size;
            // move to required member
            for (i=0; i < (position % number_of_members); i++) {
                type = member_table[tag->member_idx + i].type;
                if (type & CCHAR) {
                    index += 1;
                } else {
                    index += INTSIZE;
                }
            }
            // get value
            type = member_table[tag->member_idx + i].type;
            if (type & CCHAR) {
                result = initials_data_table[initials_data_idx + index];
            } else {
                result = (initials_data_table[initials_data_idx + index] << 8) +
                    (unsigned char)initials_data_table[initials_data_idx + index+1];
            }
        }
    }
    return result;
}
