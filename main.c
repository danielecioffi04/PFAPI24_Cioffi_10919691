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
typedef struct stockList_node stockList_node;
typedef struct lotList_node lotList_node;

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

struct lotList_node{
    ingredient content;
    int expiration;
    lotList_node* next;
};

struct stockList_node{
    lotList_node* lot;
    stockList_node* next;
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
void                    rifornimento();

//Funzioni tabella hash ricette
int                     addToRecipeHT(recipeList_node*);
int                     addToBucketRecipe(recipeList_node*, unsigned int);
unsigned int            hash(char*);

//Funzioni tabella hash magazzino
void                    addToStockHT(stockList_node*);
void                    addToBucketStock(stockList_node*, unsigned int);

//Funzioni per liste di ingredienti
void                    insertIngredientList(ingredientList_node**, ingredientList_node*);

//Funzioni per liste di lotti
void                    insertLotInOrder(lotList_node**, lotList_node*);

//Utilities
void                    elaborateCommand(char[]);
void                    printRecipeHT();
void                    printRecipeList(recipeList_node*);
void                    printIngredientList(ingredientList_node*);
void                    printStockHT();
void                    printStockLists(stockList_node*);
void                    printLotList(lotList_node*);

//------------------------------------------------------------------------------------------------------------------------------------


//Variabili globali
courier_config      courierConfig;              //Configurazioni del corriere
recipeList_node**   recipeHashTable;            //HashTable per ricette
stockList_node**      stockHashTable;             //HashTable in cui elemento è un albero di lotti ordinati per data di scadenza
int                 time = 0;                   //Istante di tempo attuale

//==============================================================================================================================
int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire

    //Inizializzazione strutture dati statiche
    recipeHashTable = (recipeList_node**)malloc(HASHTABLE_SIZE * sizeof(recipeList_node*));
    stockHashTable = (stockList_node**)malloc(HASHTABLE_SIZE * sizeof(stockList_node*));
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.maxQuantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    //Stampe per verifiche
    printStockHT();

    //Free della memoria usata per le strutture dati statiche
    free(recipeHashTable);
    free(stockHashTable);
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
            rifornimento();
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
    printf("(INGREDIENTI: ");
    printIngredientList(x->content.ingHead);
    printf(")");
    printRecipeList(x->next);
}

void printIngredientList(ingredientList_node* x){
    if(x == NULL)return;
    printf("-> %s %d", x->content.name, x->content.quantity);
    printIngredientList(x->next);
}

void printStockHT(){
    int i;
    for(i = 0 ; i < HASHTABLE_SIZE ; i++){
        if(stockHashTable[i] != NULL){
            printf("HT[%d] ", i);
            printStockLists(stockHashTable[i]);
            printf("\n");
            fflush(stdout);
        }
    }
}

void printStockLists(stockList_node* head){
    if(head == NULL) return;

    printf("==> %s \n", head->lot->content.name);
    printLotList(head->lot);
    printStockLists(head->next);
}

void printLotList(lotList_node* head){
    if(head == NULL) return;

    printf("-> %s %d %d ", head->content.name, head->content.quantity, head->expiration);
    printLotList(head->next);
}

//aggiungi_ricetta
void aggiungi_ricetta(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea

    char            name[ARG_LENGTH];                       //Nome della ricetta
    char            ingName[ARG_LENGTH];                    //Ingrediente
    unsigned int    ingQuantity;                            //Quantità ingrediente

    status = scanf("%s", name);                     //Lettura del nome della ricetta

    //Creazione ricetta
    recipeList_node* newRecipe = (recipeList_node*)malloc(sizeof(recipeList_node));
    newRecipe->content.name = (char*)malloc(strlen(name) + 1);
    strcpy(newRecipe->content.name, name);

    int res = addToRecipeHT(newRecipe);
    if(res == 0){
        printf("ignorato\n");
        return;        //Ricetta già presente
    }

    printf("aggiunta\n");
    do{
        //Lettura ingredienti
        status = scanf("%s %d", ingName, &ingQuantity);
        
        //Creazione ingrediente
        ingredientList_node* newIng = (ingredientList_node*)malloc(sizeof(ingredientList_node));
        newIng->content.name = (char*)malloc(strlen(ingName) + 1);
        strcpy(newIng->content.name, ingName);
        newIng->content.quantity = ingQuantity;

        insertIngredientList(&(newRecipe->content.ingHead), newIng);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    if(status == 0) printf("error\n");
}

int addToRecipeHT(recipeList_node* x){
    unsigned int index = hash(x->content.name);
    int res = 0;

    if(recipeHashTable[index] == NULL){
        recipeHashTable[index] = x;
        res = 1;
    }
    //Collisione
    else res = addToBucketRecipe(x, index);

    if(res == 0) return 0;
    else return 1;
}

//rifornimento
void rifornimento(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea

    char            ingName[ARG_LENGTH];
    unsigned int    ingQuantity;
    unsigned int    ingExpiration;

    do{
        //Lettura ingredienti
        status = scanf("%s %d %d", ingName, &ingQuantity, &ingExpiration);

        //Creazione lotto
        stockList_node* newItemHT = (stockList_node*)malloc(sizeof(stockList_node));
        lotList_node* newLot = (lotList_node*)malloc(sizeof(lotList_node));
        newLot->content.name = (char*)malloc(strlen(ingName) + 1);
        strcpy(newLot->content.name, ingName);
        newLot->content.quantity = ingQuantity;
        newLot->expiration = ingExpiration;

        newItemHT->lot = newLot;

        addToStockHT(newItemHT);
        
        status = scanf("%c", &eol);
    }while(eol != '\n');

    printf("rifornito\n");

    if(status == 0) printf("error\n");
}

void addToStockHT(stockList_node* x){
    unsigned int index = hash(x->lot->content.name);

    if(stockHashTable[index] == NULL) stockHashTable[index] = x;
    //Collisione
    else addToBucketStock(x, index);
}

//Funzione di hash
unsigned int hash(char* x){
    unsigned int res = 0;

    while(*x != '\0'){
        res = res * HASHPARAMETER + (unsigned char)(*x);
        x++;
    }

    return res % HASHTABLE_SIZE;
}

//Funzioni per stockHashTable
void addToBucketStock(stockList_node* x, unsigned int index){
    stockList_node* curr = stockHashTable[index];

    while(curr){
        if(strcmp(x->lot->content.name, curr->lot->content.name) == 0){
            //Trvoato ingrediente in magazzino, aggiungere lotto alla lista dei lotti
            insertLotInOrder(&(curr->lot), x->lot);
            return;
        }
        curr = curr->next;
    }

    //Ingrediente non trovato, aggiungo nuovo ingrediente nella bucketList
    x->next = stockHashTable[index];
    stockHashTable[index] = x;
}

//Funzioni per recipeHashTable
int addToBucketRecipe(recipeList_node* x, unsigned int index){
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

//Funzioni per liste di ingredienti
void insertIngredientList(ingredientList_node** head, ingredientList_node* x){
    if(*head == NULL){
        *head = x;
        return;
    }

    x->next = *head;
    *head = x;
}

//Funzioni per liste di lotti
void insertLotInOrder(lotList_node** head, lotList_node* x) {
    if (*head == NULL || x->expiration <= (*head)->expiration) {
        x->next = *head;
        *head = x;
        return;
    }

    lotList_node* curr = *head;

    while (curr && curr->next && x->expiration > curr->next->expiration) curr = curr->next;

    x->next = curr->next;
    curr->next = x;
}
