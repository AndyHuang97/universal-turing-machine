/* A nondeterministic TM simulator implementation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* ---------------------------------------------------------------------- */
typedef struct STRING{
    char **tape;    //input string;
    int length;
    int numconfig;   //num of configurations that points to this string
    //int LEFT;
}STRING;
typedef struct configuration{
    int state;          //state of the OC;
    STRING *String;
    int pos;            //position of the head on the tape(string);
    long step;
    struct configuration *next;     //for list of configurations.
}config;
typedef struct endList{     //
    char output;    //written char;
    int hmove;     //{R,L, S} head moves;
    int end;        //end state
    struct endList *next;
}endList;
typedef struct inputHash{    //Hashtable for input
    //char input;              //read char;
    //struct inputHash *next;
    //struct inputHash *prev;
    endList *end_next;
}inputHash;
typedef struct State{        //Main hashtable ordered according to states
    _Bool finalstate;               //start state
    inputHash *input_next[75];
}State;
typedef struct machine{
    State **tr;
    config *snapshot;//memorizes the OC's current state and input string's head
    long max;        //condition for termination if it's looping
}TM;

/* ---------- Private function declarations ---------- */
void init_TM(TM *);
void initInputHash(inputHash **);
endList *createEndList(char, char, int);
void ENDLIST_INSERT(endList **, endList *);
inputHash *LIST_SEARCH(State *, char);
int LoadTM(TM *);
void LIST_INSERT(State *, inputHash *);
inputHash *INPUT_SEARCH(TM , int , char );      //solo per controllo
config *MKconfig(void);
config *ND_Step(config **, config **, TM, _Bool *, _Bool *);
char *LOAD_STRING(int *);

/* ---------------------------------------------------------------------- */
void init_TM(TM *tm) {
    //printf("Initializing the TM ...\n\n");
    tm->tr = (State **)calloc(1, sizeof(State *));   //array of ptrs to State
    tm->tr[0] = (State *)calloc(1, sizeof(State));
    //tm->tr[0]->input_next = NULL;
    tm->snapshot = MKconfig();
    tm->snapshot->pos = 0;
    tm->snapshot->state = 0;
    tm->snapshot->step = 0;
    tm->snapshot->String = (STRING *)malloc(sizeof(STRING));
    tm->snapshot->String->tape = (char **)malloc(sizeof(char *));
    tm->snapshot->String->numconfig = 1;
    //tm->snapshot->String->LEFT = 0;
}

/* ---------------------------------------------------------------------- */
void initInputHash(inputHash **S) {
    *S = (inputHash *)malloc(sizeof(inputHash));
    //(*S)->input= 0;
    //(*S)->next = NULL;
    (*S)->end_next = NULL;
}

/* ---------------------------------------------------------------------- */
endList *createEndList(char output, char hmove, int end) {
    endList *new = NULL;
    new = (endList *) malloc(sizeof(endList));
    new->output = output;
    switch (hmove) {
        case 'L':
            new->hmove = -1;
            break;
        case 'R':
            new->hmove = 1;
            break;
        case 'S':
            new->hmove = 0;
            break;
    }
    new->end = end;
    new->next = NULL;
    return new;
}

/* ---------------------------------------------------------------------- */
void ENDLIST_INSERT(endList **L, endList *el) {
    if(*L == NULL)
        *L = el;
    else {
        el->next = *L;
        *L = el;
    }
}

/* ---------------------------------------------------------------------- */
endList *ENDLIST_SEARCH(endList *L, char output, char hmove, int end) {
    endList *x = L;
    int flag = 0;
    switch (hmove) {
        case 'L':
            hmove = -1;
            break;
        case 'R':
            hmove = 1;
            break;
        case 'S':
            hmove = 0;
            break;
    }
    while(x != NULL && !flag) {
        if(x->end == end && x->hmove == hmove && x->output == output)
            flag = 1;
        else
            x = x->next;
    }
    return x;
}
/* ---------------------------------------------------------------------- */
int LoadTM(TM *tm) {
    char input, output, hmove, c, Line[50];
    int start, end, states = 0, max = 0, i = 1, last_realloc_value;
    endList *new = NULL;
    
    scanf("%s", Line);
    last_realloc_value = 0;
    do {
        //reads the lines of the transition function
        //printf("input data: ");
        scanf("%d %c %c %c %d\n", &start, &input, &output, &hmove, &end);
        //printf("%d %c %c %c %d\n", start, input, output, hmove, end);
        if(max < start || max < end) {
            if(start > end)
                max = start;
            else
                max = end;
        }
        if(max > states) {//if start>states then reallocate new memory for it
            if(max > last_realloc_value) {
                last_realloc_value = max << 2;
                tm->tr = (State **)realloc(tm->tr, (last_realloc_value+2)*sizeof(State *));//realloc max+1 objects.
                //tm->tr[max+1] = NULL;//last object of dynamic array not realloced, set manually
                /*=====need to do free, as it is an array of pointers=====*/
            }
            while(i <= max) {   //just malloc for the newly added states(cells)
                //check from the last cell and allocates new memory for it. It is assumed that the first positions have already their pointers.
                tm->tr[i] = (State *)calloc(1, sizeof(State));
                //tm->tr[i]->input_next[input-48] = NULL;
                i++;
            }
            states = max;
        }
        //printf("addr: %p\n",(void *)&tm->tr[0]->input_next[0]);
        //saving transition
        if(tm->tr[start]->input_next[input-48] == NULL)
            tm->tr[start]->input_next[input-48] = (inputHash *)calloc(1, sizeof(inputHash));
        new = ENDLIST_SEARCH(tm->tr[start]->input_next[input-48]->end_next, output, hmove, end);
        if(new == NULL) {//no same transition
            new = createEndList(output, hmove, end);
            ENDLIST_INSERT(&(tm->tr[start]->input_next[input-48]->end_next), new);
        }
        c = getc(stdin);            //used to recognize the "acc" line
        ungetc(c, stdin);           //puts back the read character
    }while(c != 'a');
    //printf("Transition part completed!\n");
    scanf("%s", Line);      //"acc" line
    //printf("%s\n", Line);
    i = 0;                          //using it for accept states array
    do {
        scanf("%d\n", &i);
        tm->tr[i]->finalstate = 1;
        c = getc(stdin);            //used to recognize the "max" line
        ungetc(c, stdin);           //puts back the read character
    } while(c != 'm');
    /*for(int count = 0; count < i; count++)
     printf("%d\n", tm->Accept[count]);*/
    scanf("%s\n", Line);     //"max" line
    //printf("%s\n", Line);
    scanf("%ld\n", &tm->max);       //max steps
    //printf("%d\n", tm->max);
    scanf("%s\n", Line);   //"run" line
    //printf("%s\n", Line);
    return max;
}

/* ---------------------------------------------------------------------- */
//loading input string functions
char *LOAD_STRING(int *length){
    int dim = 1, old_dim = 1;
    char *string = (char *)malloc(2*sizeof(char));
    char c;
    
    string[0] = '_';
    string[1] = '\0';
    c = getc(stdin);
    while(c != '\n' && c != EOF)
    {
        string[dim] = c;
        dim++;
        if(old_dim < dim) {
            old_dim = 4*dim;
            string = (char *)realloc(string, (old_dim+2)*sizeof(char));
        }
        string[dim] = '_';
        c = getc(stdin);
        if(c == '\r')
            c = getc(stdin);
    }
    string[dim+1] = '\0';
    *length = dim+1;
    return string;
}


/* ---------------------------------------------------------------------- */
//initialize a new config
config *MKconfig(){
    config *c = (config *)malloc(sizeof(config));
    //c->String = (STRING *)malloc(sizeof(STRING));
    c->next = NULL;
    return c;
}

/* ---------------------------------------------------------------------- */
//adding a new confi to list
void ADDconfig(config *root, config *leaf){
    config *temp = root;
    while(temp->next != NULL)
        temp = temp->next;
    temp->next = leaf;
    //leaf->prev = temp;
}

/* ---------------------------------------------------------------------- */
void MergeConfig(config** config1_first, config **config1_last, config **config2_first, config** config2_last){
    if(*config1_last != NULL) { //costant complexity
        if(*config2_first != NULL) {
            (*config1_last)->next = *config2_first;//it is assumed that (*config1_last)->next =NULL as last config of the list
            //(*config2_first)->prev = *config1_last;
            *config1_last = *config2_last;
        }
        else
            (*config1_last)->next = NULL;
    }
    else { //(*config1_last) = NULL;
        *config1_first = *config2_first;
        *config1_last = *config2_last;
    }
}

void FREE_STRING(STRING *str){
    free(*(str->tape));
    *(str->tape) = NULL;
    free(str->tape);
    str->tape = NULL;
    free(str);
}

void LEFT(config *oneconfig) {
    //printf("strlen(oneconfig->String): %d\t L\n", oneconfig->String->length);
    if(oneconfig->pos == 0) {//pos doesn't change, remains 0
        *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->String->length+5);
        memmove(&((*(oneconfig->String->tape))[1]), *(oneconfig->String->tape), oneconfig->String->length+1);//shifts string of one position to the right.
        (*(oneconfig->String->tape))[0] = '_';
        oneconfig->String->length += 1;
    }
    else {
        if(oneconfig->pos == oneconfig->String->length-1) {
            *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->pos+5);
            (*(oneconfig->String->tape))[oneconfig->pos+1] = '_';
            (*(oneconfig->String->tape))[oneconfig->pos+2] = '\0';
            oneconfig->String->length++;
        }
        oneconfig->pos -= 1;//updates pos of one position to the left
    }
}
void RIGHT(config *oneconfig) {
    //printf("strlen(oneconfig->String): %d\t R\n", oneconfig->String->length);
    if(oneconfig->pos == 0) {//pos doesn't change, remains 0
        *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->String->length+5);
        memmove(&((*(oneconfig->String->tape))[1]), *(oneconfig->String->tape), oneconfig->String->length+1);//shifts string of one position to the right.
        (*(oneconfig->String->tape))[0] = '_';
        oneconfig->pos = 1;
        oneconfig->String->length++;
    }
    else
        if(oneconfig->pos == oneconfig->String->length-1) {
            *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->pos+5);
            (*(oneconfig->String->tape))[oneconfig->pos+1] = '_';
            (*(oneconfig->String->tape))[oneconfig->String->length+2] = '\0';
            oneconfig->String->length++;
        }
    oneconfig->pos += 1;
}
/* ---------------------------------------------------------------------- */
void ND_Transition(TM *tm) {
    //long i = 0;
    _Bool  undefined = 0, accept = 0;
    int length = (int) strlen(*(tm->snapshot->String->tape));
    config *snapshot = tm->snapshot;
    config *old_addr = NULL, *merge1 = NULL, *merge2 = NULL, *last = NULL, *last1 = NULL, *last2 = NULL;
    
    if(length <= tm->max) {
        while (!accept && tm->snapshot != NULL) {//configs = 1;
            last = NULL;
            snapshot = tm->snapshot;
            old_addr = snapshot->next;
            //printf("%lu)\n",i);
            merge1 = ND_Step(&snapshot, &last, *tm, &accept, &undefined);
            last1 = last;
            while(old_addr != NULL && !accept) {
                snapshot = old_addr;
                old_addr = snapshot->next;
                merge2 = ND_Step(&snapshot, &last, *tm, &accept, &undefined);
                last2 = last;
                MergeConfig(&merge1, &last1, &merge2, &last2);
            }
            tm->snapshot = merge1;
            /*if(!accept)
                i++;*/
        }
    }
    if(accept)
        printf("1");
    else
        if(undefined || (tm->snapshot != NULL && length > tm->max))
            printf("U");
        else
            if(tm->snapshot == NULL && !undefined)
                printf("0");
    snapshot = tm->snapshot;
    while (snapshot != NULL) {
        old_addr = snapshot->next;
        snapshot->String->numconfig--;
        if (snapshot->String->numconfig == 0) {
            FREE_STRING(snapshot->String);
        }
        free(snapshot);
        snapshot = old_addr;
    }
    //for next string, if there is any remaining
    tm->snapshot = (config *)malloc(sizeof(config)); /*needs free (not solved)*/
    tm->snapshot->String = (STRING *)malloc(sizeof(STRING));
    tm->snapshot->String->tape = (char **)malloc(sizeof(char *));
    tm->snapshot->String->numconfig = 1;
    //tm->snapshot->String->LEFT = 0;
    tm->snapshot->state = 0;
    tm->snapshot->pos = 0;
    tm->snapshot->step = 0;
    tm->snapshot->next = NULL;
}

/* ---------------------------------------------------------------------- */
config *ND_Step(config **c, config** last_config, TM tm, _Bool *accept, _Bool *undefined){
    inputHash *inputH = NULL, *inputH2 = NULL;
    config oldconfig;
    config *oneconfig = NULL, *root = NULL;
    endList *choice = NULL;
    STRING *string1 = NULL, *string2 = NULL;
    _Bool rootflag = 0, copyflag = 0, fundefined = 0;
    char inputchar;
    
    //printf("(*c)->String->tape=%s\n", *((*c)->String->tape));
    oldconfig.pos = (*c)->pos;
    oldconfig.state = (*c)->state;
    oldconfig.String = (*c)->String;
    oldconfig.step = (*c)->step;
    inputchar = (*(oldconfig.String->tape))[oldconfig.pos];
    inputH = tm.tr[oldconfig.state]->input_next[inputchar-48];
    if(inputH != NULL) {
        choice = inputH->end_next;
        string1 = oldconfig.String;
        while(choice != NULL) {      //all possible steps on the input
            //printf("BEFORE:\t%s\tS:%d\tH:%d\tnumconfig=%d\tlen=%d\tstep=%ld\n", *oldconfig.String->tape, oldconfig.state, oldconfig.pos,oldconfig.String->numconfig,oldconfig.String->length,(*c)->step);
            //FINAL STATE CHECK
            if(tm.tr[choice->end]->finalstate) {
                *accept = 1;
                //if(root != NULL) return root; else
                //return *c;  //can add control with merge_choose
            }
            if(choice->next == NULL)    //last element, recycling initial config
                oneconfig = *c;
            else
                oneconfig = MKconfig();  //creates a config var to pass to tm at the end;
            oneconfig->state = choice->end;         //update state
            oneconfig->pos = oldconfig.pos;             //update head position
            oneconfig->step = oldconfig.step;
            /* ---------------------- ONE CONFIG CASE(DET) -------------------------- */
            if(!rootflag && choice->next == NULL && oldconfig.String->numconfig == 1) {
                oneconfig->String = string1;
                /* ---------------------- CONTROL ON LOOPS -------------------------- */
                if(oldconfig.pos+1 < string1->length)
                    inputH2 = tm.tr[choice->end]->input_next[(*(string1->tape))[oldconfig.pos+1]-48];
                //1) UNIONSTUCK; 2) TOCORNOTTOC; 3) FANCYLOOPS.
                if((choice->hmove == 0 && oldconfig.state == choice->end) || (oldconfig.pos == 0 &&  oldconfig.state == choice->end && choice->hmove == -1) || (choice->hmove == 1 && inputH2 != NULL && inputH2->end_next->hmove == -1 && oldconfig.state == choice->end && (*(string1->tape))[oldconfig.pos] == choice->output  && ((*((string1)->tape))[oldconfig.pos+1] == inputH2->end_next->output) && oldconfig.state == inputH2->end_next->end)) {
                    *undefined = 1;
                    FREE_STRING(oldconfig.String);
                    oldconfig.String = NULL;
                    free(*c);
                    return NULL;
                }
                /* ---------------------- CONTROL ON DETERMISTIC MOVE -------------------------- */
                /* 1)oneconfig->pos+hmove is inside array boundaries                             */
                /* 2)char read and the next char(according di hmove) are the same                */
                /* 3)initial state is the same as the final state of transition                  */
                /* 4)char read on tape is the same as the char to write                          */
                /* 5)number of steps not over max                                                */
                /* ----------------------------------------------------------------------------- */
                while((oneconfig->pos+choice->hmove > 0 && oneconfig->pos+ choice->hmove < oldconfig.String->length-1) && (*(string1->tape))[oneconfig->pos + choice->hmove] == inputchar && oldconfig.state == choice->end && (*(string1->tape))[oneconfig->pos] == choice->output && !fundefined) {
                    oneconfig->pos += choice->hmove;
                    oneconfig->step++;
                    //printf("\t\t%s\tS:%d\tH:%d\tnumconfig=%d\tlen=%d\tstep=%ld\n", *oneconfig->String->tape, oneconfig->state, oneconfig->pos, oneconfig->String->numconfig,oneconfig->String->length, oneconfig->step);
                    if(oneconfig->step >= tm.max) {
                        fundefined = 1;
                        *undefined = 1;
                        FREE_STRING(string1);
                        string1 = NULL;
                        free(*c);//o oneconfig, è uguale
                        return NULL;
                    }
                }
                if(oneconfig->step >= tm.max) {
                    fundefined = 1;
                    *undefined = 1;
                    FREE_STRING(string1);
                    string1 = NULL;
                    free(*c);
                    return NULL;
                }
                if(inputchar != choice->output)
                    (*(oneconfig->String->tape))[oneconfig->pos] = choice->output;
                copyflag = 1;
            }
            /* ------------------- MORE THAN ONE CONFIG CASES(NON-DET) ------------------- */
            else {
                if((*(oldconfig.String->tape))[oldconfig.pos] != choice->output) {//need to change the tape
                    string2 = (STRING *)malloc(sizeof(STRING));
                    string2->tape = (char **)malloc(sizeof(char *));
                    *(string2->tape) = (char *) malloc(oldconfig.String->length+5);
                    string2->numconfig = 1;
                    //string2->LEFT = oldconfig.String->LEFT;
                    memcpy(*(string2->tape), *(oldconfig.String->tape), oldconfig.String->length+1);//copio la stringa + '\0' (+1)
                    string2->length = oldconfig.String->length;
                    oneconfig->String = string2;
                    (*(oneconfig->String->tape))[oneconfig->pos] = choice->output;   //overwrite
                }
                else {//no need to change tape
                    oneconfig->String = string1;
                    if(!copyflag)  //first time entering cycle, no need to copy
                        copyflag = 1;
                    else
                        oneconfig->String->numconfig++;
                }
            }
            /* ----------------------MOVING THE HEAD-------------------------- */
            if(choice->hmove == -1)   //move head
                LEFT(oneconfig);
            else
                if(choice->hmove == 1)
                    RIGHT(oneconfig);
            oneconfig->step++;
            //printf("AFTER:\t%s\tS:%d\tH:%d\tnumconfig=%d\tlen=%d\tstep=%ld\n", *oneconfig->String->tape, oneconfig->state, oneconfig->pos, oneconfig->String->numconfig,oneconfig->String->length, oneconfig->step);
            //linking different paths as list;
            if(!rootflag) {
                root = oneconfig;
                rootflag = 1;
            }
            else
                ADDconfig(root, oneconfig);
            choice = choice->next;
            //printf("\n");
        }
        if(!copyflag)//the initial string oldconfig->String->tape is never used in this level
            oldconfig.String->numconfig--;
        if(oldconfig.String->numconfig == 0) {
            //printf("FREED STRING:\t%s\n\n",*oldconfig.String->tape);
            FREE_STRING(oldconfig.String);
            oldconfig.String = NULL;
        }
        *last_config = oneconfig;
        return root;
    }
    else {
        //printf("STRING:\t%s\tS:%d\tH:%d\tDELETED\tnumconfig=%d\n\n",*(*c)->String->tape,(*c)->state,(*c)->pos,(*c)->String->numconfig);
        oldconfig.String->numconfig--;
        if(oldconfig.String->numconfig == 0) {
            //printf("FREED STRING:\t%s\n\n",*oldconfig.String->tape);
            FREE_STRING(oldconfig.String);
            oldconfig.String = NULL;
        }
        *last_config = NULL;
        root = (*c)->next;
        (*c)->String = NULL;
        free(*c);
        *c = root;
        return NULL;
    }
}

/* ---------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    //clock_t tStart = clock();
    char c;
    TM machine;
    
    //freopen(argv[1], "r", stdin); //check from lldb
    init_TM(&machine);
    LoadTM(&machine);
    c = getc(stdin);   //read character to see if it is EOF
    ungetc(c, stdin);   //puts back the read character
    while(c != EOF) {
        *(machine.snapshot->String->tape) = LOAD_STRING(&machine.snapshot->String->length);         //load one string
        machine.snapshot->pos = 1;  //strings have this format _"string"_, pos=1;
        ND_Transition(&machine);
        c = getc(stdin);           //needed to check the first character of the string
        if(c != EOF)
            printf("\n");
        ungetc(c, stdin);
    }
    //printf("\nTime taken: %2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    return 0;
}
