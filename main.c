#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//costanti per controllo del comando
#define ADD_RECIPE_ID       0       //Id aggiungi_ricetta
#define REMOVE_RECIPE_ID    1       //Id rimuovi_ricetta
#define ADD_ING_ID          2       //Id rifornimento
#define ADD_ORDER_ID        3       //Id ordine

//Lunghezze
#define COMMAND_LENGTH      17      //lunghezza massima di un comando preso in input
#define ARG_LENGTH          256     //Lunghezza massima di un argomento (ingredienti, ricette...) di un comando preso in input

//Dimensioni strutture dati statiche
#define HASHTABLE_SIZE      5000

//Altri parametri
#define HASHPARAMETER       31  

//Forward declaration struct
typedef struct ingredient ingredient;
typedef struct recipe recipe;
typedef struct order order;
typedef struct courier_config courier_config;
typedef struct ingredientList_node ingredientList_node;
typedef struct recipeList_node recipeList_node;

//Struct per gli elementi della pasticceria
struct ingredient{
    char* name;
    unsigned int quantity;
};

struct recipe{
    char* name;
    ingredientList_node* ingHead;
};

struct order{
    char* recipeName;
    unsigned int arrivalTime;
};

//Struct per la costruzione delle strutture dati
struct ingredientList_node{
    ingredient content;
    ingredientList_node* next;
};

struct recipeList_node{
    recipe content;
    recipeList_node* next;
};

//Configurazione corriere
struct courier_config{
    unsigned int clock;              //ogni quanto passa il corriere
    unsigned int maxQuantity;        //capienza  del corriere
};

//------------------------------------------------------------------------------------------------------------------------------------
//Prototipi
void                    aggiungi_ricetta();

//Funzioni tabella hash ricette
void                    addToRecipeHT(recipeList_node*);
int                     addToBucket(recipeList_node*, unsigned int);
unsigned int            hash(char*);

//Utilities
void                    elaborateCommand(char[]);
void                    printRecipeHT();
void                    printRecipeList(recipeList_node*);
//------------------------------------------------------------------------------------------------------------------------------------


//Variabili globali
courier_config      courierConfig;              //Configurazioni del corriere
recipeList_node**   recipeHashTable;            //HashTable per ricette
int                 time = 0;                   //Istante di tempo attuale


//==============================================================================================================================
int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire

    //Inizializzazione strutture dati statiche
    recipeHashTable = (recipeList_node**)malloc(HASHTABLE_SIZE * sizeof(recipeList_node*));
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.maxQuantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    printRecipeHT();

    //Free della memoria usata per le strutture dati statiche
    free(recipeHashTable);
}
//==============================================================================================================================

//Utilities
//Funzione che collega il comando alla rispettiva funzione da eseguire
void elaborateCommand(char command[]){
    int commandID;
    if(command[0] == 'a') commandID = ADD_RECIPE_ID;
    else if(command[0] == 'r'){
        if(command[2] == 'm') commandID = REMOVE_RECIPE_ID;
        else commandID = ADD_ING_ID;
    }
    else commandID = ADD_ORDER_ID;

    switch (commandID){
        case 0:
            //Aggiungi ricetta
            aggiungi_ricetta();
            break;
        
        case 1:
            //Rimuovi_ricetta
            break;

        case 2:
            //Rifornimento
            break;

        case 3:
            //Ordine
            break;
        
        default:
            break;
    }   
}

void printRecipeHT(){
    int i;
    for(i = 0 ; i < HASHTABLE_SIZE ; i++){
        if(recipeHashTable[i] != NULL){
            printf("HT[%d] ", i);
            printRecipeList(recipeHashTable[i]);
            printf("\n");
            fflush(stdout);
        }
    }
}

void printRecipeList(recipeList_node* x){
    if(x == NULL)return;
    printf("-> %s ", x->content.name);
    printRecipeList(x->next);
}

//aggiungi_ricetta
void aggiungi_ricetta(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea

    char            name[ARG_LENGTH];                       //Nome della ricetta
    char            ingName[ARG_LENGTH];                    //Ingrediente
    unsigned int    ingQuantity;                            //Quantità ingrediente

    status = scanf("%s", name);                     //Lettura del nome della ricetta

    do{
        //Creazione ricetta
        recipeList_node* newRecipe = (recipeList_node*)malloc(sizeof(recipeList_node));
        newRecipe->content.name = (char*)malloc(strlen(name) + 1);
        strcpy(newRecipe->content.name, name);
        addToRecipeHT(newRecipe);

        //Lettura ingredienti
        status = scanf("%s %d", ingName, &ingQuantity);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    if(status == 0) printf("error\n");
}

void addToRecipeHT(recipeList_node* x){
    unsigned int index = hash(x->content.name);
    int res = 0;

    if(recipeHashTable[index] == NULL){
        recipeHashTable[index] = x;
        res = 1;
    }
    //Collisione
    else res = addToBucket(x, index);

    if(res == 0) printf("ignorato\n");
    else printf("aggiunta\n");
}

unsigned int hash(char* x){
    unsigned int res = 0;

    while(*x != '\0'){
        res = res * HASHPARAMETER + (unsigned char)(*x);
        x++;
    }

    return res % HASHTABLE_SIZE;
}

int addToBucket(recipeList_node* x, unsigned int index){
    recipeList_node* curr = recipeHashTable[index];
    
    while(curr){
        if(strcmp(curr->content.name, x->content.name) == 0){
            return 0;
        }
        curr = curr->next;
    }

    x->next = recipeHashTable[index];
    recipeHashTable[index] = x;
    return 1;
}

