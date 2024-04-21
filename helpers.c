//
// Created by jacoblightning3 on 4/19/24.
//

#include <stdio.h>
#include "helpers.h"


void printarray(char *array[], int arraylen){
    for (int idx = 0; idx < arraylen;idx++){
        printf("Idx %i: %s\n",idx,*array++);
    }
}