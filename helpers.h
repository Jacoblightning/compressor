//
// Created by jacoblightning3 on 4/19/24.
//

#ifndef COMPRESSOR_HELPERS_H
#define COMPRESSOR_HELPERS_H

// Macros for bit manipulation
#define SET_BIT(byte, pos) ((byte) |= (1 << (pos)))
#define CLEAR_BIT(byte, pos) ((byte) &= ~(1 << (pos)))
#define TOGGLE_BIT(byte, pos) ((byte) ^= (1 << (pos)))
#define CHECK_BIT(byte, pos) ((byte) & (1 << (pos)))
#define MAX_BITS 3


void printarray(char *array[], int arraylen);


#endif //COMPRESSOR_HELPERS_H
