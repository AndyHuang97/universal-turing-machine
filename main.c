#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct configuration{
    
}config;

typedef struct transition{
    int init;
    char in;
    char out;
    char head;
    int final;
}transition;

typedef struct machine{
    transition *tr;
    int *Accept;
    config snapshot;//memorizes the OC's current state and input string's head
    int max;
}TM;

void Load_MT(FILE);

int main(int argc, const char * argv[]) {
    char input[25] = "input.txt";
    
    char *Line = (char *)malloc(sizeof(char)); //line ha un indirizzo, quindi ci posso salvare stringhe
    transition *tr = (transition *) malloc(sizeof(transition));
    //int i;
    char *car = (char *)malloc(sizeof(char));
    FILE *fp;
    
    printf("input file: %s\n", input);
    fp = fopen(input, "r");
    if(fp == NULL)
        printf("Unable to open input file\n");
    else
    {
        printf("Success in opening input file!\n");
        fgets(Line, 100, fp);       ////legge tr;
        printf("%s",Line);
        /*
        for(i = 0; i < 3; i++)      //legge tr;
        {
            Let = fgetc(fp);
            if(Let == '\n')
                printf("beccato newline!\n");
            printf("%d - %c\n", i, Let);
        }*/
        //Let = fgetc(fp);  //legge primo stato (int)
        fgets(Line, 100, fp);
        while(strcmp(Line, "acc\n")){       //mancaono tutti gli hashtables
            *car = *Line;
            tr->init = atoi(car);       //int
            printf("-%d", tr->init);
            tr->in = *(Line + 2);
            tr->out = *(Line + 4);
            tr->head = *(Line + 6);
            *car = *(Line + 8);
            tr->final = atoi(car);     //int
            printf("-%d ", tr->final);
            printf("%s", Line);
            fgets(Line, 100, fp);
        }

        printf("END\n");
    }
    return 0;
}
