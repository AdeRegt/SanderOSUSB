#include "../include/string.h"

int strcmp(const char *str1, const char *str2){
    int p = 0;
    while(1){
        char A = str1[p];
        char B = str2[p];
        p++;
        if(A==0&&B==0){
            break;
        }else if(A==0||B==0){
            return -1;
        }else if(A!=B){
            return 1;
        }
    }
    return 0;
}