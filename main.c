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
typedef struct courierList_node courierList_node;

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
    unsigned int weight;
    ingredientList_node* ingList;
    recipeList_node* next;
    recipeList_node* prev;
};

struct orderList_node{
    char* name;
    unsigned int arrivalTime;
    unsigned int quantity;
    unsigned int weight;
    unsigned int suspended;
    orderList_node* next;
    orderList_node* prev;
    ingredientList_node* ingList;
};

struct courierList_node{
    char* name;
    unsigned int arrivalTime;
    unsigned int quantity;
    unsigned int weight;
    courierList_node* next;
    courierList_node* prev;
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
void                    corriere();

//Funzione di hash
unsigned int            hash(char*);

//Funzioni tabella hash ricette
void                    insertRecipe(recipeList_node*);
recipeList_node*        searchRecipe(char*);
void                    removeRecipe(recipeList_node**, recipeList_node*);

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
unsigned int            fullfillOrder(orderList_node*);
void                    enqueueCompletedOrder(orderList_node*);
void                    enqueueSuspendedOrder(orderList_node*);
orderList_node*         removeSuspendedOrder(orderList_node**, orderList_node*);
void                    insertCompletedOrder(orderList_node**, orderList_node*);
orderList_node*         removeCompletedOrder(orderList_node**);
void                    insertOrderToShip(courierList_node**, courierList_node*);

//Utilities
void                    elaborateCommand(char[]);
void                    printAllRecipes();
void                    printRecipeList(recipeList_node*);
void                    printIngredientList(ingredientList_node*);
void                    printAllStocks();
void                    printStockList(stockList_node*);
void                    printLotList(lotList_node*);
void                    printOrderList(orderList_node*);
void                    printCourierList(courierList_node*);

//------------------------------------------------------------------------------------------------------------------------------------


//Variabili globali
courier_config      courierConfig;              //Configurazioni del corriere
recipeList_node**   recipeHashTable;            //HashTable per ricette
stockList_node**    stockHashTable;             //HashTable in cui elemento è un albero di lotti ordinati per data di scadenza

orderList_node*     completedOrders = NULL;
orderList_node*     lastCompleted = NULL;

orderList_node*     suspendedOrders = NULL;
orderList_node*     lastSuspended = NULL;

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
                removeRecipe(&recipeHashTable[i], curr);
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

    while(completedOrders){
        orderList_node* tmp = completedOrders;
        completedOrders = completedOrders->next;
        free(tmp);
    }

    while(suspendedOrders){
        orderList_node* tmp = suspendedOrders;
        suspendedOrders = suspendedOrders->next;
        free(tmp);
    }
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

    if(time % courierConfig.clock == 0 && time != 0){
        corriere();
    }

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
    printf("=> %s WEIGHT: %d", x->name, x->weight);
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

void printOrderList(orderList_node* x){
    if(x == NULL){
        printf("\n");
        return;
    }

    if(x->next && x->next->prev != x) printf(" (Errore) ");
    if(lastCompleted == x || lastSuspended == x) printf("-> %d S:%d - %s WEIGHT: %d LAST", x->arrivalTime, x->suspended, x->name, x->weight);
    else printf("-> %d S:%d - %s WEIGHT: %d ", x->arrivalTime, x->suspended, x->name, x->weight);
    printOrderList(x->next);
}

void printCourierList(courierList_node* x){
    if(x == NULL){
        printf("\n");
        return;
    }

    if(x->next && x->next->prev != x) printf(" (Errore) ");
    else printf("-> %d - %s WEIGHT: %d ", x->arrivalTime, x->name, x->weight);
    printCourierList(x->next);
}

//corriere
void corriere(){
    courierList_node* list = NULL;
    orderList_node* curr = completedOrders;
    int max = courierConfig.maxQuantity;

    while(max > 0 && curr){
        if(curr->weight > max) break;
        orderList_node* next = curr->next;

        max -= curr->weight;
        orderList_node* removed = removeCompletedOrder(&completedOrders);
        if(removed->next != NULL || removed->prev != NULL) printf("Errore nella rimozione\n");

        courierList_node* newNode = (courierList_node*)malloc(sizeof(courierList_node));
        newNode->next = NULL;
        newNode->prev = NULL;
        newNode->name = removed->name;
        newNode->arrivalTime = removed->arrivalTime;
        newNode->weight = removed->weight;
        newNode->quantity = removed->quantity;

        free(removed);

        insertOrderToShip(&list, newNode);

        curr = next;
    }

    if(list == NULL){
        printf("camioncino vuoto\n");
        return;
    }

    while(list){
        courierList_node* tmp = list;
        printf("%d %s %d\n", list->arrivalTime, list->name, list->quantity);

        list = list->next;
        free(tmp);
    }
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
    x->weight   = 0;

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

        x->weight += quantity;

        insertIngredient(&(x->ingList), y);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    printf("aggiunta\n");

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
    int status;
    char    name[ARG_LENGTH];

    status = scanf("%s", name);

    recipeList_node* recipe = searchRecipe(name);

    if(recipe == NULL){
        printf("non presente\n");
        return;
    }

    orderList_node* curr = suspendedOrders;

    while(curr){
        if(strcmp(curr->name, name) == 0){
            printf("ordini in sospeso\n");
            return;
        }
        curr = curr->next;
    }

    curr = completedOrders;
    while(curr){
        if(strcmp(curr->name, name) == 0){
            printf("ordini in sospeso\n");
            return;
        }
        curr = curr->next;
    }

    printf("rimossa\n");
    removeRecipe(&recipeHashTable[hash(name)], recipe);

    if(searchRecipe(name) != NULL) printf("Ricetta non rimossa correttamente\n");

    if(status == 0) printf("error\n");
}

void removeRecipe(recipeList_node** head, recipeList_node* x){
    //Sto cercando di eliminare la testa
    if(*head == x){
        *head = x->next;
        if(*head) (*head)->prev = NULL;

        x->next = NULL;
        x->prev = NULL;
        free(x->name);
        removeIngredients(&(x->ingList));
        free(x);
        return;
    }

    //Sono in mezzo alla lista
    x->prev->next = x->next;
    if(x->next) x->next->prev = x->prev;

    x->next = NULL;
    x->prev = NULL;
    free(x->name);
    removeIngredients(&(x->ingList));
    free(x);
    return;
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

    orderList_node* currOrder = suspendedOrders;
    while(currOrder){
        unsigned int res = fullfillOrder(currOrder);

        orderList_node* nextOrder = currOrder->next;
        
        //L'ordine sospeso è stato completato
        if(res == 1){
            orderList_node* completed = removeSuspendedOrder(&suspendedOrders, currOrder);
            if(completed->next != NULL || completed->prev != NULL) printf("Error: nodo non correttamente rimosso\n");
            insertCompletedOrder(&completedOrders, completed);
        }
        currOrder = nextOrder;
    }

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

    //Se ho eliminato tutta la lista
    if(stock->lotHead == NULL){
        stock->lotHead = x;
        return;
    }

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
    while(stock->lotHead && (stock->lotHead->expiration <= time || stock->lotHead->quantity == 0)){
        stock->tot -= stock->lotHead->quantity;

        lotList_node* tmp = stock->lotHead;
        stock->lotHead = stock->lotHead->next;
        if(stock->lotHead) stock->lotHead->prev = NULL;

        free(tmp);
    }
}

//ordine
void ordine(){
    int             status;
    char            name[ARG_LENGTH];
    unsigned int    quantity;

    status = scanf("%s %d", name, &quantity);
    recipeList_node* recipe = searchRecipe(name);

    if(recipe == NULL){
        printf("rifiutato\n");
        return;
    }

    printf("accettato\n");
    orderList_node* x = (orderList_node*)malloc(sizeof(orderList_node));
    x->name = recipe->name;
    x->quantity = quantity;
    x->suspended = 0;
    x->arrivalTime = time;
    x->weight = recipe->weight * quantity;
    x->ingList = recipe->ingList;
    x->next = NULL;
    x->prev = NULL;

    unsigned int res = fullfillOrder(x);
    if(res == 0){
        enqueueSuspendedOrder(x);
    }
    else {
        enqueueCompletedOrder(x);
    }

    if(status == 0) printf("error\n");
}

unsigned int fullfillOrder(orderList_node* x){ //0 se sospeso, 1 se completato
    ingredientList_node* currIng = x->ingList;

    while(currIng){
        unsigned int ingRequestedQuantity = currIng->quantity * x->quantity;

        stockList_node* stock = searchStock(currIng->name);
        if(stock) removeUselessLots(stock); //Elimino i lotti scaduti e vuoti e così aggiorno stock->tot
        if(stock == NULL || stock->tot < ingRequestedQuantity){
            //Sospendo l'ordine
            x->suspended = 1;
            return 0;
        }
        currIng = currIng->next;
    }

    //Se sono qui allora posso completare l'ordine
    x->suspended = 0;

    currIng = x->ingList;
    while(currIng){
        unsigned int ingRequestedQuantity = currIng->quantity * x->quantity;

        stockList_node* stock = searchStock(currIng->name);
        if(stock == NULL || stock->tot < ingRequestedQuantity) printf("ERROR - Stock is null or not enough ingredients\n");

        lotList_node* currLot = stock->lotHead;
        while(currLot){
            //Lotto attuale può soddisfare la richiesta di ingrediente
            if(currLot->quantity >= ingRequestedQuantity){
                stock->tot -= ingRequestedQuantity;
                currLot->quantity -= ingRequestedQuantity;
                ingRequestedQuantity = 0;
                
                break;
            }

            //Lotto attuale non può soddisfare la richiesta di ingrediente
            stock->tot -= currLot->quantity;
            ingRequestedQuantity -= currLot->quantity;
            currLot->quantity = 0;

            currLot = currLot->next;
        }

        currIng = currIng->next;
    }

    return 1;
}

//Aggiunge ordine completato in coda (si presuppone che questo ordine non sia collegato a nessuna lista)
void enqueueCompletedOrder(orderList_node* x){
    //Lista vuota
    if(lastCompleted == NULL){
        completedOrders = x;
        lastCompleted = x;
        return;
    }

    //Lista non vuota
    lastCompleted->next = x;
    x->prev = lastCompleted;
    lastCompleted = x;
}

//Aggiunge in ordine
void insertCompletedOrder(orderList_node** head, orderList_node* x) {
    //Lista vuota
    if (*head == NULL) {
        *head = x;
        lastCompleted = x;
        return;
    }

    //Inserimento in coda
    if (x->arrivalTime >= lastCompleted->arrivalTime) {
        lastCompleted->next = x;
        x->prev = lastCompleted;
        lastCompleted = x;
        return;
    }

    //Inserimento in testa
    if (x->arrivalTime <= (*head)->arrivalTime) {
        (*head)->prev = x;
        x->next = *head;
        *head = x;
        return;
    }

    orderList_node* curr = *head;
    while (curr->next && x->arrivalTime > curr->next->arrivalTime) {
        curr = curr->next;
    }

    //Inserimento dopo curr e prima di curr->next
    x->next = curr->next;
    if (curr->next) curr->next->prev = x;
    else lastCompleted = x;
    curr->next = x;
    x->prev = curr;
}

//Aggiunge ordine sospeso in coda (si presuppone che questo ordine non sia collegato a nessuna lista)
void enqueueSuspendedOrder(orderList_node* x){
    //Lista vuota
    if(lastSuspended == NULL){
        suspendedOrders = x;
        lastSuspended = x;
        return;
    }

    //Lista non vuota
    lastSuspended->next = x;
    x->prev = lastSuspended;
    lastSuspended = x;
}

orderList_node* removeSuspendedOrder(orderList_node** head, orderList_node* x){
    if(lastSuspended == x) lastSuspended = lastSuspended->prev;
    
    //Sto cercando di eliminare la testa
    if(*head == x){
        *head = x->next;
        if(*head) (*head)->prev = NULL;

        x->next = NULL;
        x->prev = NULL;
        return x;
    }

    //Sono in mezzo alla lista
    x->prev->next = x->next;
    if(x->next) x->next->prev = x->prev;

    x->next = NULL;
    x->prev = NULL;
    return x;
}

orderList_node* removeCompletedOrder(orderList_node** head){
    if(lastCompleted == *head) lastCompleted = NULL;

    orderList_node* res = *head;
    *head = (*head)->next;
    if(*head) (*head)->prev = NULL;

    res->next = NULL;
    res->prev = NULL;
    return res;
}

//Aggiunge in ordine decrescente di peso
void insertOrderToShip(courierList_node** head, courierList_node* x) {
    if(*head == NULL){
        *head = x;
        return;
    }

    if (x->weight > (*head)->weight || (x->weight == (*head)->weight && x->arrivalTime < (*head)->arrivalTime)) {
        x->next = *head;
        (*head)->prev = x;
        x->prev = NULL;
        *head = x;
        return;
    }

    courierList_node* curr = *head;

    while(curr->next && (x->weight < curr->next->weight || (x->weight == curr->next->weight && x->arrivalTime > curr->next->arrivalTime))){
        curr = curr->next;
    }

    x->next = curr->next;
    if(curr->next) curr->next->prev = x;
    curr->next = x;
    x->prev = curr;

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