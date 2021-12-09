/*      I/O Practice File       */
#include <stdio.h>
#include <stdlib.h>

int main() {
    int c;
    FILE *in, *out;

    in = fopen("filein.txt","r");
    out = fopen("fileout.txt","w");
    while((c = fgetc(in)) != EOF){
        fputc(c,out);
    }
    exit(0);
}