#include "../kernel.h"

int pow(int base,int exp){
	unsigned long value=1;
	while (exp!=0){
      		value*=base;
      		--exp;
  	}
  	return value;
}
