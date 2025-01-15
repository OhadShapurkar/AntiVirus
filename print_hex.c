#include <stdio.h>
#include <stdlib.h>

void printHex(char* buffer, int size)
{
    for(int i = 0; i < size && buffer[i]; i++)
    {
        printf("%X ", (unsigned char)buffer[i]);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    int X = 13;
    FILE* infile = fopen(argv[1], "r");
    if(infile == NULL)
    {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    char buffer[X];
    while(fread(buffer, 1, X, infile) != 0)
    {
        printHex(buffer, X);
    }
    return 0;
}