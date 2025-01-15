#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define BUFFER_SIZE 10000

typedef struct virus 
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};

void printAscii(char* buffer, int size, FILE* output)
{
    for(int i = 0; i < size; i++)
    {
        fprintf(output, "%c", buffer[i]);
        
    }
}


void printHex(unsigned char* buffer, int size, FILE* output)
{
    for(int i = 0; i < size; i++)
    {
        fprintf(output, "%.2X ", buffer[i]);
    }
}

bool compareCharArray(char* str1, char* str2, int size)
{
    for(int i = 0; i < size; i++)
    {
        if(str1[i] != str2[i])
        {
            return false;
        }
    }
    return true;
}

void printDec(int num, FILE* output)
{
    fprintf(output, "%d ", num);
}


link* list_append(link* virus_list, virus* data)
{
    link* newLink = (link*)malloc(sizeof(link));
    if(newLink == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(newLink);
        exit(EXIT_FAILURE);
    }
    newLink->vir = data;
    newLink->nextVirus = virus_list;
    return newLink;
}

void list_free(link *virus_list)
{
    link* current = virus_list;
    link* temp;
    while(current != NULL)
    {
        temp = current;
        current = current->nextVirus;
        free(temp->vir->sig);
        free(temp->vir);
        free(temp);
    }
}

virus* readVirus(FILE* file, bool isLittleEndian)
{
    virus* v = (virus*)malloc(sizeof(virus));
    if(v == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(v);
        exit(EXIT_FAILURE);
    }
    char buffer[18];
    size_t result = fread(buffer, 1, 18, file);
    if(result != 18)
    {
        free(v);
        return NULL;
    }
    if(isLittleEndian)
    {
        v->SigSize = (unsigned short)(buffer[0] | (buffer[1] << 8));
    }
    else
    {
        v->SigSize = (unsigned short)((buffer[0] << 8) | buffer[1]);
    }
    for(int i = 2; i < 18; i++)
    {
        v->virusName[i - 2] = buffer[i];
    }
    v->sig = (unsigned char*)malloc(v->SigSize);
    if(v->sig == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(v);
        exit(EXIT_FAILURE);
    }
    result = fread(v->sig, 1, v->SigSize, file);
    if(result == 0)
    {
        fprintf(stderr, "Error: Cannot read from file2\n");
        free(v->sig);
        free(v);
        exit(EXIT_FAILURE);
    }
    return v;
}

void printVirus(virus* virus, FILE* output)
{
    fprintf(output, "Virus name: ");
    printAscii(virus->virusName, 16, output);
    fprintf(output, "\nVirus size: ");
    printDec(virus->SigSize, output);
    fprintf(output, "\nsignature:\n");
    printHex(virus->sig, virus->SigSize, output);
    printf("\n\n");
}

void list_print(link *virus_list, FILE* output)
{
    link* current = virus_list;
    while(current != NULL)
    {
        printVirus(current->vir, output);
        current = current->nextVirus;
    }
}

void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    link* current = virus_list;
    while(current != NULL)
    {
        for(int i = 0; i < size; i++)
        {
            if(memcmp(buffer + i, current->vir->sig, current->vir->SigSize) == 0)
            {
                printf("Starts at byte %d\n", i);
                printf("Virus detected: ");
                printVirus(current->vir, stdout);
            }
        }
        current = current->nextVirus;
    }
}

void neutralize_virus(char *fileName, int signatureOffset)
{
    FILE* file = fopen(fileName, "r+");
    if(file == NULL)
    {
        fprintf(stderr, "Error: Cannot open file\n");
    }
    else
    {
        fseek(file, signatureOffset, SEEK_SET);
        unsigned char buffer[1] = {0xC3};
        fwrite(buffer, 1, 1, file);
        fclose(file);
    }
}

void fix_file(char* filename, link* virus_list)
{
    FILE* file = fopen(filename, "r");
    if(file == NULL)
    {
        fprintf(stderr, "Error: Cannot open file\n");
    }
    else
    {
        fseek(file, 0, SEEK_END);
        unsigned int size = (unsigned int)ftell(file);
        fseek(file, 0, SEEK_SET);
        char buffer[BUFFER_SIZE] = {0};
        fread(buffer, 1, size, file);
        fclose(file);
        link* current = virus_list;
        while(current != NULL)
        {
            for(int i = 0; i < size; i++)
            {
                if(memcmp(buffer + i, current->vir->sig, current->vir->SigSize) == 0)
                {
                    neutralize_virus(filename, i);
                }
            }
            current = current->nextVirus;
        }
    }
}


int main(int argc, char** argv)
{
    FILE* out = stdout;
    link* virus_list = NULL;
    int choice = -1;
    bool isLittleEndian = false;
    FILE* file;
    while(!feof(stdin) && choice != 5)
    {
        char buffer[BUFFER_SIZE] = {0};
        printf("Menu:\n1) Load signatures\n2) Print signatures\n3) Detect viruses\n4) Fix file\n5) Quit\n");
        fgets(buffer, BUFFER_SIZE, stdin);
        sscanf(buffer, "%d", &choice);
        switch (choice)
        {
        case 1:
            printf("Enter file name:\n");
            fflush(stdin);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            FILE* signatures = fopen(buffer, "r");
            if(signatures == NULL)
            {
                fprintf(stderr, "Error: Cannot open file\n");
            }
            else
            {
                fread(buffer, 1, 4, signatures);
                if(!compareCharArray(buffer, "VIRL", 4) && !compareCharArray(buffer, "VIRB", 4))
                {
                    fprintf(stderr, "Error: File is not a virus signatures file\n");
                    fclose(signatures);
                }
                if(compareCharArray(buffer, "VIRL", 4))
                {
                    isLittleEndian = true;
                }
                else
                {
                    isLittleEndian = false;
                }
                while(1)
                {
                    virus* v = readVirus(signatures, isLittleEndian);
                    if(v == NULL)
                    {
                        break;
                    }
                    virus_list = list_append(virus_list, v);
                }
                fclose(signatures);
            }
            break;
        case 2:
            list_print(virus_list, out);
            break;
        case 3:
            printf("Enter file name:\n");
            fflush(stdin);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            file = fopen(buffer, "r");
            if(file == NULL)
            {
                fprintf(stderr, "Error: Cannot open file\n");
            }
            else
            {
                fseek(file, 0, SEEK_END);
                unsigned int size = (unsigned int)ftell(file);
                fseek(file, 0, SEEK_SET);
                fread(buffer, 1, size, file);
                detect_virus(buffer, size, virus_list);
                fclose(file);
            }
            break;
        case 4:
            printf("Enter file name:\n");
            fflush(stdin);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            char* filename = buffer;
            file = fopen(buffer, "r");
            if(file == NULL)
            {
                fprintf(stderr, "Error: Cannot open file\n");
            }
            else
            {
                fix_file(filename, virus_list);
                fclose(file);
            }
            break;
        case 5:
            break;
        default:
            if(!feof(stdin))
                fprintf(stderr, "Error: Invalid choice\n");
            break;
        }
        
    }
    list_free(virus_list);
    exit(EXIT_SUCCESS);

    
    return 0;
}