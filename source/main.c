#include <stdio.h>
#include "controller.h"

Elev myElevator;


int main() {
    
    printf("Run program\n");
	controller_run(&myElevator);

    return 0;
}
