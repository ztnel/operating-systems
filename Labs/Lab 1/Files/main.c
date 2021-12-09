/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include "a.h"

extern void function_two(); 
extern void function_three(); 

int main(){
    printf ("%s","main:\n");
    function_two();
    function_three();
    exit (EXIT_SUCCESS);
}