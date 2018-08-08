/* A nondeterministic TM simulator implementation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* ---------------------------------------------------------------------- */
typedef struct STRING{
    char **tape;                    //input string;
    int length;                     //length of the string
    int numconfig;                  //num of configurations that points to this string
}STRING;
typedef struct configuration{
    int state;                      //state of the OC;
    STRING *String;
    int pos;                        //position of the head on the tape(string);
    long step;                      //
    struct configuration *next;     //for list of configurations.
}config;
typedef struct endList{             //stores the ACTUAL TRANSITION
    char output;                    //written char;
    int hmove;                      //{R,L, S} head moves;
    int end;                        //end state
    struct endList *next;
}endList;
typedef struct inputHash{
    endList *end_next;
}inputHash;
typedef struct State{
    _Bool finalstate;               //accepting state or not
    inputHash *input_next[75];      // n. of ASCII characters ranging from '0' to 'z', can be changed to 128 to contain every ASCII standard printable character
}State;
typedef struct machine{
    State **tr;                     //array of pointers to State
    config *snapshot;               //memorizes the OC's current state and input string's head
    long max;                       //condition for undefined termination
}TM;

/* ---------- Private function declarations ---------- */
endList *createEndList(char, char, int);
void ENDLIST_INSERT(endList **, endList *);
endList *ENDLIST_SEARCH(endList *, char, char, int);
int LoadTM(TM *);
char *LOAD_STRING(int *);
config *MKconfig(void);
void ADDconfig(config *, config *);
void MergeConfig(config **, config **, config **, config **);
void FREE_STRING(STRING *);
void LEFT(config *);
void RIGHT(config *);
void ND_Transition(TM *);
config *ND_Step(config **, config **, TM, _Bool *, _Bool *);

/* ---------------------------------------------------------------------- */
//creates an endList variable/transition given output, hmove and end
endList *createEndList(char output, char hmove, int end) {
    endList *new = NULL;
    new = (endList *) malloc(sizeof(endList));
    new->output = output;
    switch (hmove) {                                        //converting {L, S, R} to {-1, 0, 1}
        case 'L':
            new->hmove = -1;
            break;
        case 'S':
            new->hmove = 0;
            break;
        case 'R':
            new->hmove = 1;
            break;
    }
    new->end = end;
    new->next = NULL;
    return new;
}

/* ---------------------------------------------------------------------- */
//inserts transition as head of the given list
void ENDLIST_INSERT(endList **L, endList *el) {
    if(*L == NULL)  //first time inserting
        *L = el;
    else {          //inserting as head of list
        el->next = *L;
        *L = el;
    }
}

/* ---------------------------------------------------------------------- */
//searches the transition in the given list *L, returns the address of the element if present, NULL otherwise
endList *ENDLIST_SEARCH(endList *L, char output, char hmove, int end) {
    endList *x = L;
    int flag = 0;
    switch (hmove) {                            //converting {L, S, R} to {-1, 0, 1}
        case 'L':
            hmove = -1;
            break;
        case 'S':
            hmove = 0;
            break;
        case 'R':
            hmove = 1;
            break;
        }
    while(x != NULL && !flag) {                 //scans the list for the transition
        if(x->end == end && x->hmove == hmove && x->output == output)
            flag = 1;
        else
            x = x->next;
    }
    return x;       //returns a non-NULL value if the transition is already present in the list
}
/* ---------------------------------------------------------------------- */
//loads the transitions, accepting states, and max depth; returns max number of states
int LoadTM(TM *tm) {
    char input, output, hmove, c, Line[50];
    int start, end, states = 0, max = 0, i = 1, last_realloc_value;
    endList *new = NULL;
    
    scanf("%s", Line);              //"tr" line
    last_realloc_value = 0;
    do {                            /* read the lines of the transition function */
        //printf("input data: ");
        scanf("%d %c %c %c %d\n", &start, &input, &output, &hmove, &end);
        //printf("%d %c %c %c %d\n", start, input, output, hmove, end);
        if(max < start || max < end) {//max is the greates number of state between starting and ending state
            if(start > end)
                max = start;
            else
                max = end;
        }
        if(max > states) {          //if start>states then reallocate new memory for it
            if(max > last_realloc_value) {
                last_realloc_value = max << 2;
                tm->tr = (State **)realloc(tm->tr, (last_realloc_value+2)*sizeof(State *));//realloc 4*max objects.
            }
            while(i <= max) {       //malloc for the newly added states(cells)
                                    //check from the last cell and allocates new memory for it. It is assumed that the first positions have already their pointers.
                tm->tr[i] = (State *)calloc(1, sizeof(State));
                i++;
            }
            states = max;
        }
        /* saving transition */
        if(tm->tr[start]->input_next[input-48] == NULL) //not added input character
            tm->tr[start]->input_next[input-48] = (inputHash *)calloc(1, sizeof(inputHash));
        new = ENDLIST_SEARCH(tm->tr[start]->input_next[input-48]->end_next, output, hmove, end);
        if(new == NULL) {           //no same transition, so it needs insertion
            new = createEndList(output, hmove, end);
            ENDLIST_INSERT(&(tm->tr[start]->input_next[input-48]->end_next), new);
        }
        c = getc(stdin);            //used to recognize the "acc" line
        ungetc(c, stdin);           //puts back the read character
    }while(c != 'a');
    //printf("Transition part completed!\n");
    scanf("%s", Line);              //"acc" line
    do {
        scanf("%d\n", &i);          //reading the accepting state
        tm->tr[i]->finalstate = 1;  //updating finalstate field
        c = getc(stdin);            //used to recognize the "max" line
        ungetc(c, stdin);           //puts back the read character
    } while(c != 'm');
    scanf("%s\n", Line);            //"max" line
    scanf("%ld\n", &tm->max);       //max steps
    scanf("%s\n", Line);            //"run" line
    return max;                     //returns the max number of states
}

/* ---------------------------------------------------------------------- */
//load input string, returns the string
char *LOAD_STRING(int *length){
    int dim = 1, old_dim = 1;
    char *string = (char *)malloc(2*sizeof(char));      //string of 2 bytes: "_\0"
    char c;
    
    string[0] = '_';
    string[1] = '\0';
    c = getc(stdin);
    while(c != '\n' && c != EOF)
    {
        string[dim] = c;                //updates last position
        dim++;
        if(old_dim < dim) {             //the dimension of the dynamic array rises as a power of 4
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
    return string;                      //returning the string
}


/* ---------------------------------------------------------------------- */
//initialize a new config
config *MKconfig(){
    config *c = (config *)malloc(sizeof(config));
    c->next = NULL;
    return c;
}

/* ---------------------------------------------------------------------- */
//(could have added it as head) add  a new config to the end of the list, mainly because it's easier to read when printing out the list of transitions later in the computation
void ADDconfig(config *root, config *leaf){
    config *temp = root;
    while(temp->next != NULL)
        temp = temp->next;
    temp->next = leaf;
}

/* ---------------------------------------------------------------------- */
//merge with constant complexity two lists of configurations
void MergeConfig(config** config1_first, config **config1_last, config **config2_first, config** config2_last) {
    if(*config1_last != NULL) {                         //first list is not empty
        if(*config2_first != NULL) {                    //second list is not epmty
            (*config1_last)->next = *config2_first;     //it is assumed that (*config1_last)->next =NULL as last config of the list
            *config1_last = *config2_last;              //update the last config of the whole list
        }
        else                                            //second list is empty
            (*config1_last)->next = NULL;
    }
    else { //first list is empty
        *config1_first = *config2_first;                //updates first config of the whole list
        *config1_last = *config2_last;                  //update the last config of the whole list
    }
}

/* ---------------------------------------------------------------------- */
//free a pointer to structure STRING
void FREE_STRING(STRING *str){
    free(*(str->tape));
    *(str->tape) = NULL;
    free(str->tape);
    str->tape = NULL;
    free(str);
}

/* ---------------------------------------------------------------------- */
//move che head of the tape to left
void LEFT(config *oneconfig) {
    //printf("strlen(oneconfig->String): %d\t L\n", oneconfig->String->length);
    if(oneconfig->pos == 0) {                                       //pos doesn't change, remains 0
        *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->String->length+5);
        memmove(&((*(oneconfig->String->tape))[1]), *(oneconfig->String->tape), oneconfig->String->length+1);                           //shifts string of one position to the right.
        (*(oneconfig->String->tape))[0] = '_';
        oneconfig->String->length++;
    }
    else {
        if(oneconfig->pos == oneconfig->String->length-1) {         //arrived at utmost right '_'
            *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->pos+5);
            (*(oneconfig->String->tape))[oneconfig->pos+1] = '_';   //update last two positions
            (*(oneconfig->String->tape))[oneconfig->pos+2] = '\0';
            oneconfig->String->length++;
        }
        oneconfig->pos--;                                           //updates pos of one position to the left
    }
}
//move che head of the tape to right
void RIGHT(config *oneconfig) {
    //printf("strlen(oneconfig->String): %d\t R\n", oneconfig->String->length);
    if(oneconfig->pos == 0) {                                       //pos becomes 2 as we add a '_' to the left
        *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->String->length+5);
        memmove(&((*(oneconfig->String->tape))[1]), *(oneconfig->String->tape), oneconfig->String->length+1);                           //shifts string of one position to the right.
        (*(oneconfig->String->tape))[0] = '_';                      //update first position
        oneconfig->pos = 1;
        oneconfig->String->length++;
    }
    else
        if(oneconfig->pos == oneconfig->String->length-1) {         //arrived at utmost right '_'
            *(oneconfig->String->tape) = (char *)realloc(*(oneconfig->String->tape), oneconfig->pos+5);
            (*(oneconfig->String->tape))[oneconfig->pos+1] = '_';   //update last two positions
            (*(oneconfig->String->tape))[oneconfig->pos+2] = '\0';
            oneconfig->String->length++;
        }
    oneconfig->pos++;
}

/* ---------------------------------------------------------------------- */
//executes the nondeterministic steps of the computation and merges the resulting lists as one; prints the output
void ND_Transition(TM *tm) {
    //long i = 0;
    _Bool  undefined = 0, accept = 0;
    int length = (int) strlen(*(tm->snapshot->String->tape));
    config *snapshot = tm->snapshot;
    config *old_addr = NULL, *merge1 = NULL, *merge2 = NULL, *last = NULL, *last1 = NULL, *last2 = NULL;
    
    if(length <= tm->max) { //if length of the string is greater than max depth then it's undefined
        while (!accept && tm->snapshot != NULL) {//
            last = NULL;
            snapshot = tm->snapshot;
            old_addr = snapshot->next;          //needed to take account of next config in the list
            //printf("%lu)\n",i);
            merge1 = ND_Step(&snapshot, &last, *tm, &accept, &undefined);   //merge1 = head of the first list of configs
            last1 = last;                       //last1 = last config of the first list of configs
            while(old_addr != NULL && !accept) {
                snapshot = old_addr;
                old_addr = snapshot->next;
                merge2 = ND_Step(&snapshot, &last, *tm, &accept, &undefined);   //merge2 = head of the second list of configs
                last2 = last;                   //last2 = last config of the second list of configs
                MergeConfig(&merge1, &last1, &merge2, &last2);  //the first list becomes the merged list
            }
            tm->snapshot = merge1;              //updates the list of configs for the tm
            /*if(!accept)
                i++;*/
        }
    }
    if(accept)
        printf("1");
    else
        if(undefined || (tm->snapshot != NULL && length > tm->max)) //step or length exceeded the ma depth
            printf("U");
        else
            if(tm->snapshot == NULL && !undefined)  //there are no configs left and no paths exceeded max depth
                printf("0");
    snapshot = tm->snapshot;
    while (snapshot != NULL) {
        old_addr = snapshot->next;              //needed to take account of next config in the list
        snapshot->String->numconfig--;          //decreases the amount of configs using the string
        if (snapshot->String->numconfig == 0) { //deleting the string
            FREE_STRING(snapshot->String);
        }
        free(snapshot);
        snapshot = old_addr;
    }
}

/* ---------------------------------------------------------------------- */
// starting from the configuration *c, gets the transitions and adds the new configurations; returns the first config of the list if there is at least one transition available, otherwise returns NULL, when there is no transition available or a condition for undefined is met(loop or max depth exceeded) and consequently *c is deleted.
config *ND_Step(config **c, config** last_config, TM tm, _Bool *accept, _Bool *undefined){
    inputHash *inputH = NULL, *inputH2 = NULL;
    config oldconfig;
    config *oneconfig = NULL, *root = NULL;
    endList *choice = NULL;
    STRING *string1 = NULL, *string2 = NULL;
    _Bool rootflag = 0, copyflag = 0, fundefined = 0;
    char inputchar;
    
    //printf("(*c)->String->tape=%s\n", *((*c)->String->tape));
    oldconfig.pos = (*c)->pos;                  //saving *c datas as we will recycle it if needed(nondetermistic)
    oldconfig.state = (*c)->state;
    oldconfig.String = (*c)->String;
    oldconfig.step = (*c)->step;
    inputchar = (*(oldconfig.String->tape))[oldconfig.pos];     //accessing the character on the tape
    inputH = tm.tr[oldconfig.state]->input_next[inputchar-48];  //retrieving the transition
    if(inputH != NULL) {
        choice = inputH->end_next;
        string1 = oldconfig.String;                             //getting the initial string
        while(choice != NULL) {                                 //all possible transitions on the input
            //printf("BEFORE:\t%s\tS:%d\tH:%d\tnumconfig=%d\tlen=%d\tstep=%ld\n", *oldconfig.String->tape, oldconfig.state, oldconfig.pos,oldconfig.String->numconfig,oldconfig.String->length,(*c)->step);
            /* FINAL STATE CHECK */
            if(tm.tr[choice->end]->finalstate)                  //checking accepting state
                *accept = 1;
            /* oneconfig is the support variable we will use */
            if(choice->next == NULL)                            //last node to add to list, recycling initial config
                oneconfig = *c;
            else
                oneconfig = MKconfig();                         //creates a config var to pass to tm at the end;
            oneconfig->state = choice->end;                     //update state
            oneconfig->pos = oldconfig.pos;                     //get head position
            oneconfig->step = oldconfig.step;                   //get step
            /* ---------------------- ONE CONFIG CASE(DET) -------------------------- */
            /* there is only one transition possibile and only one config using the string */
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
                /* ---------------------- CONTROL ON DETERMISTIC MOVE ----------- */
                /* 1)oneconfig->pos+hmove is inside array boundaries              */
                /* 2)char read and the next char(according di hmove) are the same */
                /* 3)initial state is the same as the final state of transition   */
                /* 4)char read on tape is the same as the char to write           */
                /* 5)number of steps not over max                                 */
                /* control needed to avoid repeated access to transition on same  */
                /* consecutive input, i.e. string "aaaabbbb" on first 'a' and 'b' */
                /* -------------------------------------------------------------- */
                while((oneconfig->pos+choice->hmove > 0 && oneconfig->pos+ choice->hmove < oldconfig.String->length-1) && (*(string1->tape))[oneconfig->pos + choice->hmove] == inputchar && oldconfig.state == choice->end && (*(string1->tape))[oneconfig->pos] == choice->output && !fundefined) {
                    oneconfig->pos += choice->hmove;
                    oneconfig->step++;
                    //printf("\t\t%s\tS:%d\tH:%d\tnumconfig=%d\tlen=%d\tstep=%ld\n", *oneconfig->String->tape, oneconfig->state, oneconfig->pos, oneconfig->String->numconfig,oneconfig->String->length, oneconfig->step);
                    if(oneconfig->step >= tm.max) { //step surpasses the max depth possible, activated in while
                        fundefined = 1;
                        *undefined = 1;
                        FREE_STRING(string1);
                        string1 = NULL;
                        free(*c);
                        return NULL;
                    }
                }
                if(oneconfig->step >= tm.max) {     //step surpasses the max depth possible, activated when no while is needed
                    fundefined = 1;
                    *undefined = 1;
                    FREE_STRING(string1);
                    string1 = NULL;
                    free(*c);
                    return NULL;
                }
                if(inputchar != choice->output)     //overwrite the character on tape if needed
                    (*(oneconfig->String->tape))[oneconfig->pos] = choice->output;
                copyflag = 1;                       //needed to maintain the numconfig of the string
            }
            /* ------------------- MORE THAN ONE CONFIG CASES(NON-DET) ------------------- */
            else {
                if((*(oldconfig.String->tape))[oldconfig.pos] != choice->output) {  //need to change the tape, because the string on tape is changed
                    string2 = (STRING *)malloc(sizeof(STRING));
                    string2->tape = (char **)malloc(sizeof(char *));
                    *(string2->tape) = (char *) malloc(oldconfig.String->length+5);
                    string2->numconfig = 1;
                    memcpy(*(string2->tape), *(oldconfig.String->tape), oldconfig.String->length+1);  //copio la stringa + '\0' (+1)
                    string2->length = oldconfig.String->length;
                    oneconfig->String = string2;
                    (*(oneconfig->String->tape))[oneconfig->pos] = choice->output;  //overwrite
                }
                else {                                                              //no need to change tape
                    oneconfig->String = string1;
                    if(!copyflag)                                                   //first time entering cycle, no need to copy
                        copyflag = 1;
                    else
                        oneconfig->String->numconfig++;
                }
            }
            /* ----------------------MOVING THE HEAD-------------------------- */
            if(choice->hmove == -1)                 //move head to left
                LEFT(oneconfig);
            else
                if(choice->hmove == 1)              //move head to right
                    RIGHT(oneconfig);
            oneconfig->step++;
            //printf("AFTER:\t%s\tS:%d\tH:%d\tnumconfig=%d\tlen=%d\tstep=%ld\n", *oneconfig->String->tape, oneconfig->state, oneconfig->pos, oneconfig->String->numconfig,oneconfig->String->length, oneconfig->step);
            if(!rootflag) {                         //control for first node given by the transition
                root = oneconfig;
                rootflag = 1;
            }
            else
                ADDconfig(root, oneconfig);         //linking different nodes given by transition as list;
            choice = choice->next;
            //printf("\n");
        }
        if(!copyflag)                               //the initial string oldconfig->String->tape is never used in this level
            oldconfig.String->numconfig--;          //decrease the n. of configs ponting to the initial string
        if(oldconfig.String->numconfig == 0) {      //delete the string
            //printf("FREED STRING:\t%s\n\n",*oldconfig.String->tape);
            FREE_STRING(oldconfig.String);
            oldconfig.String = NULL;
        }
        *last_config = oneconfig;                   //updates poiter to last config of list
        return root;
    }
    else {                                          //no transition for the input in the current state, deleting the configuration
        //printf("STRING:\t%s\tS:%d\tH:%d\tDELETED\tnumconfig=%d\n\n",*(*c)->String->tape,(*c)->state,(*c)->pos,(*c)->String->numconfig);
        oldconfig.String->numconfig--;              //decrease the n. of configs ponting to the string
        if(oldconfig.String->numconfig == 0) {      //delete the string
            //printf("FREED STRING:\t%s\n\n",*oldconfig.String->tape);
            FREE_STRING(oldconfig.String);
            oldconfig.String = NULL;
        }
        *last_config = NULL;                        //no element in the list
        (*c)->String = NULL;
        free(*c);
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
    machine.tr = (State **)calloc(1, sizeof(State *));          //array of ptrs to State
    machine.tr[0] = (State *)calloc(1, sizeof(State));
    LoadTM(&machine);
    c = getc(stdin);                                //read character to see if it is EOF
    ungetc(c, stdin);                               //puts back the read character
    while(c != EOF) {
        machine.snapshot = (config *)calloc(1, sizeof(config));
        machine.snapshot->String = (STRING *)malloc(sizeof(STRING));
        machine.snapshot->String->tape = (char **)malloc(sizeof(char *));
        machine.snapshot->String->numconfig = 1;
        machine.snapshot->pos = 1;                  //strings have this format _"string"_, pos=1;
        *(machine.snapshot->String->tape) = LOAD_STRING(&machine.snapshot->String->length); //load one string
        ND_Transition(&machine);
        c = getc(stdin);                            //needed to check the first character of the string
        if(c != EOF)
            printf("\n");
        ungetc(c, stdin);
    }
    //printf("\nTime taken: %2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    return 0;
}
