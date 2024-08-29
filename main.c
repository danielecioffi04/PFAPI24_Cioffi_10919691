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
#define HASHPARAMETER       47  

//Forward declaration struct
typedef struct courier_config courier_config;
typedef struct ingredientList_node ingredientList_node;
typedef struct recipeList_node recipeList_node;
typedef struct stockList_node stockList_node;
typedef struct lotList_node lotList_node;
typedef struct orderList_node orderList_node;

//Struct per la costruzione delle strutture dati
struct ingredientList_node{
    char* name;
    unsigned int quantity;
    ingredientList_node* next;
};

struct lotList_node{
    unsigned int quantity;
    unsigned int expiration;
    lotList_node* next;
    lotList_node* prev;
};

struct stockList_node{
    char* name;
    unsigned int tot;
    lotList_node* lotHead;
    stockList_node* next;
};

struct recipeList_node{
    char* name;
    ingredientList_node* ingList;
    recipeList_node* next;
    recipeList_node* prev;
};

struct orderList_node{
    char* name;
    unsigned int arrivalTime;
    unsigned int quantity;
    unsigned int expiration;
    unsigned int weight;
    unsigned int suspended;
    orderList_node* next;
    orderList_node* prev;
};

//Configurazione corriere
struct courier_config{
    unsigned int clock;              //ogni quanto passa il corriere
    unsigned int maxQuantity;        //capienza  del corriere
};

//------------------------------------------------------------------------------------------------------------------------------------
//Prototipi
void                    aggiungi_ricetta();
void                    rimuovi_ricetta();
void                    rifornimento();
void                    ordine();

//Funzione di hash
unsigned int            hash(char*);

//Funzioni tabella hash ricette
void                    insertRecipe(recipeList_node*);
recipeList_node*        searchRecipe(char*);
void                    removeRecipe(recipeList_node**);

//Funzioni tabella hash magazzino
void                    insertStock(stockList_node*);
stockList_node*         searchStock(char*);

//Funzioni per liste di ingredienti
void                    insertIngredient(ingredientList_node**, ingredientList_node*);
void                    removeIngredients(ingredientList_node**);

//Funzioni per liste di lotti
void                    insertLot(stockList_node*, lotList_node*);
void                    removeUselessLots(stockList_node*);

//Funzioni per liste di ordini

//Utilities
void                    elaborateCommand(char[]);
void                    printAllRecipes();
void                    printRecipeList(recipeList_node*);
void                    printIngredientList(ingredientList_node*);
void                    printAllStocks();
void                    printStockList(stockList_node*);
void                    printLotList(lotList_node*);

//------------------------------------------------------------------------------------------------------------------------------------


//Variabili globali
courier_config      courierConfig;              //Configurazioni del corriere
recipeList_node**   recipeHashTable;            //HashTable per ricette
stockList_node**    stockHashTable;             //HashTable in cui elemento è un albero di lotti ordinati per data di scadenza
int                 time = 0;                   //Istante di tempo attuale


//==============================================================================================================================
int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire

    //Inizializzazione strutture dati statiche
    recipeHashTable = (recipeList_node**)malloc(HASHTABLE_SIZE * sizeof(recipeList_node*));
    stockHashTable = (stockList_node**)malloc(HASHTABLE_SIZE * sizeof(stockList_node*));

    for(int i=0 ; i<HASHTABLE_SIZE ; i++){
        recipeHashTable[i] = NULL;
        stockHashTable[i] = NULL;
    }
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.maxQuantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    //Stampe per verifiche

    //Liberazione memoria
    for(int i=0 ; i<HASHTABLE_SIZE ; i++){
        if(recipeHashTable[i]){
            recipeList_node* curr = recipeHashTable[i];
            recipeList_node* tmp = NULL;

            while(curr){
                tmp = curr->next;
                removeRecipe(&curr);
                curr = tmp;
            }
        }
    }
    free(recipeHashTable);
    
    for(int i=0 ; i<HASHTABLE_SIZE ; i++){
        if(stockHashTable[i]){
            stockList_node* curr = stockHashTable[i];
            while(curr){
                stockList_node* tmp = curr;
                while(curr->lotHead){
                    lotList_node* tmp = curr->lotHead;
                    curr->lotHead = curr->lotHead->next;
                    if(curr->lotHead) curr->lotHead->prev = NULL;
                    free(tmp);
                }
                curr = curr->next;
                free(tmp->name);
                free(tmp);
            }
        }
    }
    
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
            time++;
            break;
        
        case 1:
            //Rimuovi_ricetta
            rimuovi_ricetta();
            time++;
            break;

        case 2:
            //Rifornimento
            rifornimento();
            time++;
            break;

        case 3:
            //Ordine
            ordine();
            time++;
            break;
        
        default:
            break;
    }   
}

void printAllRecipes(){
    unsigned int i = 0;
    for(i=0 ;i < HASHTABLE_SIZE ; i++){
        if(recipeHashTable[i]){
            printf("RECIPE - ENTRY[%d]:\n", i);
            printRecipeList(recipeHashTable[i]);
        }
    }
}

void printRecipeList(recipeList_node* x){
    if(x == NULL){
        printf("\n");
        return;
    }
    if(x->next && x->next->prev != x) printf(" (Errore) ");
    printf("=> %s ", x->name);
    printf(" (Ingredienti: ");
    printIngredientList(x->ingList);
    printRecipeList(x->next);
}

void printIngredientList(ingredientList_node* x){
    if(x == NULL){
        printf(")");
        return;
    } 
    printf("-> %s %d ", x->name, x->quantity);
    printIngredientList(x->next);
}

void printAllStocks(){
    unsigned int i = 0;
    for(i=0 ; i < HASHTABLE_SIZE ; i++){
        if(stockHashTable[i]){
            printf("%d - STOCK - ENTRY[%d]:\n", time, i);
            printStockList(stockHashTable[i]);
        }
    }
}

void printStockList(stockList_node* x){
    if(x == NULL){
        printf("\n");
        return;
    }
    printf("stock: %s %d ", x->name, x->tot);
    printf("\n");
    printLotList(x->lotHead);
    printStockList(x->next);
}

void printLotList(lotList_node* x){
    if(x == NULL){
        printf("\n");
        return;
    }

    if(x->next && x->next->prev != x) printf(" (Errore) ");
    printf("-> %d EXP:%d ", x->quantity, x->expiration);
    printLotList(x->next);
}

//aggiungi_ricetta
void aggiungi_ricetta(){
    int             status;
    char            eol = '0'; //end of line
    char            name[ARG_LENGTH];
    char            ingName[ARG_LENGTH];
    unsigned int    quantity;

    status = scanf("%s", name);
    if(searchRecipe(name) != NULL){
        printf("ignorato\n");
        do{
            status = scanf("%s %d", ingName, &quantity);
            status = scanf("%c\n", &eol);
        }while(eol != '\n');
        
        return;
    }

    //Creazione nodo ricetta
    recipeList_node* x = (recipeList_node*)malloc(sizeof(recipeList_node));
    x->name = (char*)malloc(strlen(name) + 1);
    strcpy(x->name, name);
    x->ingList  = NULL;
    x->next     = NULL;
    x->prev     = NULL;

    //Inserimento ricetta
    insertRecipe(x);

    do{
        status = scanf("%s %d", ingName, &quantity);

        //Creazione ingrediente
        ingredientList_node* y = (ingredientList_node*)malloc(sizeof(ingredientList_node));
        y->name = (char*)malloc(strlen(ingName) + 1);
        strcpy(y->name, ingName);
        y->quantity = quantity;
        y->next = NULL;

        insertIngredient(&(x->ingList), y);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    printf("accettato\n");

    if(status == 0) printf("error\n");
    
}

void insertRecipe(recipeList_node* x){
    unsigned int index = hash(x->name);

    //Entry vuota
    if(recipeHashTable[index] == NULL){
        recipeHashTable[index] = x;
        return;
    }

    //Collisione - inserimento in testa
    x->next = recipeHashTable[index];
    recipeHashTable[index]->prev = x;
    recipeHashTable[index] = x;
}

void insertIngredient(ingredientList_node** head, ingredientList_node* x){
    if(*head == NULL){
        *head = x;
        return;
    }

    //Inserimento in testa
    x->next = *head;
    *head = x;
}

recipeList_node* searchRecipe(char* x){
    unsigned int index = hash(x);

    if(recipeHashTable[index] == NULL) return NULL;
    
    recipeList_node* curr = recipeHashTable[index];
    while(curr){
        if(strcmp(curr->name, x) == 0) return curr;
        curr = curr->next;
    }

    return NULL;
}

//rimuovi_ricetta
void rimuovi_ricetta(){

}

void removeRecipe(recipeList_node** x){
    recipeList_node* tmp = *x;

    //Testa della lista
    if((*x)->prev == NULL){
        *x = (*x)->next;
        if(*x) (*x)->prev = NULL;

        free(tmp->name);
        removeIngredients(&(tmp->ingList));
        free(tmp);
        return;
    }

    //In mezzo alla lista
    recipeList_node* prev = (*x)->prev;
    recipeList_node* next = (*x)->next;
    (*x)->prev->next = next;
    if((*x)->next) (*x)->next->prev = prev;

    free(tmp->name);
    removeIngredients(&(tmp->ingList));
    free(tmp);
}

void removeIngredients(ingredientList_node** x){
    while(*x){
        ingredientList_node* tmp = *x;
        *x = (*x)->next;
        free(tmp->name);
        free(tmp);
    }
}

//rifornimento
void rifornimento(){
    int             status;
    char            eol = '0';
    char            name[ARG_LENGTH];
    unsigned int    quantity;
    unsigned int    expiration;

    do{
        status = scanf("%s %d %d", name, &quantity, &expiration);
        lotList_node* lot = (lotList_node*)malloc(sizeof(lotList_node));
        lot->expiration = expiration;
        lot->quantity = quantity;
        lot->next = NULL;
        lot->prev = NULL;

        stockList_node* res = searchStock(name);
        if(res != NULL){
            insertLot(res, lot);
        }
        else{
            //Creazione nuovo stock
            stockList_node* stock = (stockList_node*)malloc(sizeof(stockList_node));
            stock->name = (char*)malloc(strlen(name) + 1);
            strcpy(stock->name, name);
            stock->tot = 0;
            stock->lotHead = lot;
            stock->next = NULL;

            insertStock(stock);
        }

        status = scanf("%c", &eol);
    }while(eol != '\n');

    printf("rifornito\n");

    if(status == 0) printf("error\n");
}

void insertStock(stockList_node* x){
    x->tot += x->lotHead->quantity;

    unsigned int index = hash(x->name);

    //Entru vuota
    if(stockHashTable[index] == NULL){
        stockHashTable[index] = x;
        return;
    }

    //Collisione - inserimento in testa
    x->next = stockHashTable[index];
    stockHashTable[index] = x;
}

stockList_node* searchStock(char* x){
    unsigned int index = hash(x);

    if(stockHashTable[index] == NULL) return NULL;

    stockList_node* curr = stockHashTable[index];
    while(curr){
        if(strcmp(curr->name, x) == 0) return curr;
        curr = curr->next;
    }

    return NULL;
}

void insertLot(stockList_node* stock, lotList_node* x){
    stock->tot += x->quantity; //Aggiorna tot

    if(stock->lotHead == NULL){
        stock->lotHead = x;
        return;
    }

    //Eliminazione lotti scaduti
    removeUselessLots(stock);

    //Se x scade prima della testa
    if(stock->lotHead && x->expiration < stock->lotHead->expiration){
        x->next = stock->lotHead;
        stock->lotHead->prev = x;
        stock->lotHead = x;
        return;
    }

    lotList_node* curr = stock->lotHead;

    //Scorro fino a che non trovo il lotto da eliminare
    while(curr->next && x->expiration > curr->next->expiration){
        curr = curr->next;
    }

    //Siamo in mezzo alla lista
    if(curr->next){
        x->next = curr->next;
        curr->next->prev = x;
        curr->next = x;
        x->prev = curr;
        return;
    }

    //Siamo in fondo alla lista
    curr->next = x;
    x->prev = curr;
}

//Rimuove tutti i lotti scaduti o con 0 quantità
void removeUselessLots(stockList_node* stock){
    while(stock->lotHead->expiration <= time || stock->lotHead->quantity == 0){
        stock->tot -= stock->lotHead->quantity;

        lotList_node* tmp = stock->lotHead;
        stock->lotHead = stock->lotHead->next;
        if(stock->lotHead) stock->lotHead->prev = NULL;

        free(tmp);
    }
}

//ordine
void ordine(){

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