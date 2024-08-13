#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//costanti per controllo del comando
#define ADD_RECIPE_ID       0       //Id aggiungi_ricetta
#define REMOVE_RECIPE_ID    1       //Id rimuovi_ricetta
#define ADD_ING_ID          2       //Id rifornimento
#define ADD_ORDER_ID        3       //Id ordine

//Lunghezze
#define COMMAND_LENGTH      16      //lunghezza massima di un comando preso in input
#define ARG_LENGTH          255     //Lunghezza massima di un argomento (ingredienti, ricette...) di un comando preso in input

//Forward declaration struct
typedef struct ingredient ingredient;
typedef struct ingredient_node ingredient_node;
typedef struct recipe recipe;
typedef struct order order;
typedef struct stockItem stockItem;
typedef struct courier_config courier_config;
typedef struct recipe_node recipe_node;
typedef struct stock_node stock_node;
typedef struct lot_node lot_node;

//Struct per gli elementi della pasticceria
struct ingredient{
    char name[ARG_LENGTH];
    int quantity;
};

struct recipe{
    char name[ARG_LENGTH];
    ingredient_node* ingList;
};

struct order{
    recipe recipe;
    int arrivalTime;
};

struct stockItem{
    ingredient ing;
    lot_node* head;
};

//Struct per la costruzione delle strutture dati
struct ingredient_node{         //Usato per liste di ingredienti nelle ricette
    ingredient content;
    ingredient_node* next;
};

struct recipe_node{             //Usato per costruire albero di ricette
    recipe content;
    recipe_node* left;
    recipe_node* right;
    recipe_node* parent;
    char color;
};

struct stock_node{              //Usato per costruire albero del magazzino
    char name[ARG_LENGTH];
    stock_node* parent;
    stock_node* right;
    stock_node* left;
    lot_node* lotHead;
    char color;
};

struct lot_node{                //Usato per costruire albero dei lotti
    int expiration;
    int quantity;
    lot_node* left;
    lot_node* right;
    lot_node* parent;
};

//Configurazione corriere
struct courier_config{
    int clock;              //ogni quanto passa il corriere
    int maxQuantity;        //capienza  del corriere
};

//Prototipi---------------------------------------------------------------------------------------------------------------------------
void    add_recipe();
void    rifornimento();

//Utilities
void    printRecipeTree(recipe_node*, int);
void    printStockTree(stock_node*, int);
void    elaborateCommand(char[]);

//Funzioni per albero ricette
int     insertRecipe(recipe_node**, recipe_node*);
void    leftRotateRecipe(recipe_node**, recipe_node*);
void    rightRotateRecipe(recipe_node**, recipe_node*);
void    insertRecipeFixUp(recipe_node**, recipe_node*);

//Funzioni per albero magazzino
void    insertStockItem(stock_node**, stock_node*);
void    leftRotateStockItem(stock_node**, stock_node*);
void    rightRotateStockItem(stock_node**, stock_node*);
void    insertStockItemFixUp(stock_node**, stock_node*);

//Funzioni per gestione delle liste di ingredienti
void    addToList(ingredient_node**, ingredient_node*);
void    printList(ingredient_node*);

//------------------------------------------------------------------------------------------------------------------------------------

//Variabili globali
int             time = 0;                   //Istante di tempo attuale
courier_config  courierConfig;              //Configurazioni del corriere
recipe_node*    recipeList = NULL;          //Albero delle ricette
stock_node*     stock = NULL;               //Albero magazzino

//MAIN ===============================================================================================================================
int main(){
    int             status;                     //Var in cui si conserva il valore dell'ultimo scanf
    char            command[COMMAND_LENGTH];    //Comando corrente da eseguire
    
    //Lettura dell'input
    status = scanf("%d %d\n", &courierConfig.clock, &courierConfig.maxQuantity); //Lettura della configurazione del corriere
    do{
        status = scanf("%s", command);
        if(status == 1) elaborateCommand(command);
    }while(status == 1);

    //printRecipeTree(recipeList, 0);
    //printStockTree(stock, 0);
    return 1;
}
//====================================================================================================================================

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
            //Aggiungi_ricetta
            add_recipe();
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

//Stampa un albero in modo "carino"
void printRecipeTree(recipe_node* node, int depth) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    if(node != recipeList && node->parent->left != NULL && node == node->parent->left) printf("L - %s - %c\n", node->content.name, node->color);
    else printf("R - %s - %c\n", node->content.name, node->color);

    printRecipeTree(node->left, depth + 1);

    printRecipeTree(node->right, depth + 1);
}

void printStockTree(stock_node* node, int depth){
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    if(node != stock && node->parent->left != NULL && node == node->parent->left) printf("L - %s - %c\n", node->name, node->color);
    else printf("R - %s - %c\n", node->name, node->color);

    printStockTree(node->left, depth + 1);

    printStockTree(node->right, depth + 1);
}

//Stampa lista
void printList(ingredient_node* head){
    while(head != NULL){
        printf("-> %s %d ", head->content.name, head->content.quantity);
        head = head->next;
    }
}

//Funzioni
void add_recipe(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea
    int     res;                                    //Risultato dell'aggiunta della ricetta all'albero

    //Allocazione e inizializzazione nuova ricetta da inserire nell'albero
    recipe_node* newNode = (recipe_node*)malloc(sizeof(recipe_node));
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->parent = NULL;
    newNode->content.ingList = NULL;

    //Lettura nome della ricetta e aggiunta del nodo all'albero se non presente
    status = scanf("%s", newNode->content.name);                    //Lettura del nome della ricetta
    res = insertRecipe(&recipeList, newNode);

    //Stampa risultato aggiunta
    if(res == 0){
        printf("ignorato\n");
        free(newNode);
        return;
    }
    else printf("aggiunta\n");

    //Lettura args  (TODO! CREARE LISTA DEGLI INGREDIENTI)
    do{
        ingredient_node* newIng = (ingredient_node*)malloc(sizeof(ingredient_node));
        ingredient tmp;
        status = scanf(" %s %d", tmp.name, &tmp.quantity);
        newIng->content = tmp;
        addToList(&newNode->content.ingList, newIng);

        status = scanf("%c", &eol);
    }while(eol != '\n');

    if(status == 0) printf("hello");
}

void rifornimento(){
    int     status;                                 //Var in cui si conserva il valore dell'ultimo scanf;
    char    eol = '0';                              //Var per controllare la fine della linea
    int     expiration;                             //Var che contiene la data di scadenza del lotto corrente
    //int     quantity;                               //Var che contiene la quantità del lotto corrente

    //Lettura args
    do{
        stock_node* newNode = (stock_node*)malloc(sizeof(stock_node));

        status = scanf(" %s %d %d", newNode->name, &expiration, &expiration);
        insertStockItem(&stock, newNode);
        status = scanf("%c", &eol);
    }while(eol != '\n');

    printf("rifornito\n");

    if(status == 0) printf("Error");
}

//Funzioni albero ricette
int insertRecipe(recipe_node** head, recipe_node* newNode){
    recipe_node* x = *head;
    recipe_node* y = NULL;

    while(x != NULL){
        y = x;
        int cmp = strcmp(newNode->content.name, x->content.name);
        if(cmp < 0) x = x->left;
        else if(cmp == 0) return 0;       //Ricetta già presente in lista
        else x = x->right;
    }

    newNode->parent = y;
    if(y == NULL){
        *head = newNode;        //Albero vuoto
    }
    else if(strcmp(newNode->content.name, y->content.name) < 0) y->left = newNode;
    else y->right = newNode;
    newNode->color = 'r';
    insertRecipeFixUp(head, newNode);
    return 1;
}

void insertRecipeFixUp(recipe_node** head, recipe_node* node){
    if(node == (*head)){
        node->color = 'b';
        return;
    }

    recipe_node* x = node->parent;          //Padre
    recipe_node* y = NULL;                  //Nonno
    recipe_node* z = NULL;                  //Zio
    
    if(x != NULL) y = x->parent;

    if(x->color == 'r'){
        if(y != NULL && x == y->left){
            z = y->right;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertRecipeFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->right){
                    node = x;
                    leftRotateRecipe(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                rightRotateRecipe(head, x->parent);
            }
        }
        else{
            z = y->left;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertRecipeFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->left){
                    node = x;
                    rightRotateRecipe(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                leftRotateRecipe(head, x->parent);
            }
        }
    }
}

void leftRotateRecipe(recipe_node** head, recipe_node* node){
    recipe_node* y = node->right;
    node->right = y->left;

    if(y->left != NULL){
        y->left->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->left) node->parent->left = y;
    else node->parent->right = y;
    y->left = node;
    node->parent = y;
}

void rightRotateRecipe(recipe_node** head, recipe_node* node){
    recipe_node* y = node->left;
    node->left = y->right;
    
    if(y->right != NULL){
        y->right->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->right) node->parent->right = y;
    else node->parent->left = y;
    y->right = node;
    node->parent = y;
}

//Funzioni albero magazzino
void insertStockItem(stock_node** head, stock_node* newNode){
    stock_node* x = *head;
    stock_node* y = NULL;

    while(x != NULL){
        y = x;
        int cmp = strcmp(newNode->name, x->name);
        if(cmp < 0) x = x->left;
        else if(cmp == 0){
            printf("Ingrediente già presente\n");
            return;
        }
        else x = x->right;
    }

    newNode->parent = y;
    if(y == NULL){
        *head = newNode;        //Albero vuoto
    }
    else if(strcmp(newNode->name, y->name) < 0) y->left = newNode;
    else y->right = newNode;
    newNode->color = 'r';
    insertStockItemFixUp(head, newNode);
}

void insertStockItemFixUp(stock_node** head, stock_node* node){
    if(node == (*head)){
        node->color = 'b';
        return;
    }

    stock_node* x = node->parent;          //Padre
    stock_node* y = NULL;                  //Nonno
    stock_node* z = NULL;                  //Zio
    
    if(x != NULL) y = x->parent;

    if(x->color == 'r'){
        if(y != NULL && x == y->left){
            z = y->right;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertStockItemFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->right){
                    node = x;
                    leftRotateStockItem(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                rightRotateStockItem(head, x->parent);
            }
        }
        else{
            z = y->left;           //y è lo zio di node
            //CASO 1
            if(z != NULL && z->color == 'r'){
                x->color = 'b';
                z->color = 'b';
                y->color = 'r';
                insertStockItemFixUp(head, y);
            }
            //CASO 2
            else{
                if(node == x->left){
                    node = x;
                    rightRotateStockItem(head, node);
                    x = node->parent;
                    y = x->parent;
                }
                //CASO 3
                x->color = 'b';
                x->parent->color = 'r';
                leftRotateStockItem(head, x->parent);
            }
        }
    }
}

void leftRotateStockItem(stock_node** head, stock_node* node){
    stock_node* y = node->right;
    node->right = y->left;

    if(y->left != NULL){
        y->left->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->left) node->parent->left = y;
    else node->parent->right = y;
    y->left = node;
    node->parent = y;
}

void rightRotateStockItem(stock_node** head, stock_node* node){
    stock_node* y = node->left;
    node->left = y->right;
    
    if(y->right != NULL){
        y->right->parent = node;
    }
    y->parent = node->parent;
    if(node->parent == NULL) *head = y;
    else if(node == node->parent->right) node->parent->right = y;
    else node->parent->left = y;
    y->right = node;
    node->parent = y;
}



//Funzioni su liste di ingredienti
void addToList(ingredient_node** head, ingredient_node* newNode){       //Inserimento in testa
    if(*head == NULL) {
        *head = newNode;
        return;
    }

    ingredient_node* tmp = *head;
    *head = newNode;
    newNode->next = tmp;
}