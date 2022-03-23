#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define RAM_TEST_ARRAY_SIZE 16
#define MRAM_SIZE 16
#define CACHE_SIZE 4 
#define BLOCK_SIZE 2
#define NULL_VALUE_FOR_CACHE -111
// standard values for constants: 
// RAM_TEST_ARRAY_SIZE = 1000
// MRAM_SIZE = 1024
// CACHE_SIZE = 32
// BLOCK_SIZE = 4



// problema: 
// ram = 0 -> 123
//       1 -> 456

// accesa o espaco 0 e atualiza ele de 123 para 789 e coloca na cache
// cache : 789 456
// entao acessa o espaco 0 denovo e procura pro 123 na cache, ve que nao tem
// e coloca o novo bloco 123 456 na cache dnv
// cache : 789 456
// cache : 123 456


// 2 structs usados para a criacao da FIFO, 
typedef struct Item {
    int value;
    struct Item* Next;
}Item;
typedef struct Fila {
    int lenght;
    Item* First;
    Item* Last;
}Fila;

// usado para a criacao de array para o teste automatico
typedef struct InputObj{
    int value;
    int update;
}InputObj;

// usado para a memoria cache, onde a cache se trata de 
// um array de N CacheSlot's, cada CacheSlot eh um array de BLOCK_SIZE CacheValue's
// cada CacheSlot se iguala eh um bloco da cache, que guarda em si um certo numero
// de valores definidos pelo BLOCK_SIZE
typedef struct CacheValue{
    int value;
    int RAMaddress;
    int update;
}CacheValue;
typedef struct CacheSlot{
    CacheValue values[BLOCK_SIZE];
}CacheSlot;




int getRandomInt(int low, int high){
        return rand() % (high-low) + low;
}
Item* initializeItem(Item* i){
    i -> value = 0;
    i -> Next = NULL;
    return i;
}
Fila* initializeFila(Fila* f){
    f->lenght = 0;
    f->First = NULL;
    f->Last = NULL;
    return f;
}
int insertFila(Item* new, Fila* f){
    if (f->lenght == 0){
        f->First = new;
        f->Last = new;
        f->lenght++;
        return 1;
    }
    new -> Next = f -> First;
    f -> First = new;
    f -> lenght++;
    return 1;
}
Item* RemoveLast(Fila* f){
    if (f -> lenght == 0){
        return NULL;
    }

    Item* r = f -> Last;

    if (f -> lenght == 1){
        f -> First = NULL;
        f -> Last = NULL;
        f->lenght--;
        return r;
    }

    Item* t = f -> First;
    while(t -> Next -> Next != NULL){
        t = t -> Next;
    }
    f -> Last = t;
    t -> Next = NULL;
    f->lenght--;

    return r;
}
Item* NextInLine(Fila* fila){
    if (fila->lenght == 0){
        return NULL;
    }
    Item* t = RemoveLast(fila);
    insertFila(t, fila);
    return t;
}
int printFila(Fila* f){
    if (f-> First == NULL){
        printf("\nNo items in list");
        return 0;
    }
    Item* t = f -> First;
    printf("\n");
    while (t -> Next != NULL){
        printf(" %d |", t -> value);
        t = t -> Next;
    }
    printf(" %d", t -> value);
    printf("  <-- Proximo Valor");
    
    return 1;
}
Fila* randomizeFila(Fila* fila){
    int iter = 0;
    while (iter < CACHE_SIZE){
        Item* cell = malloc( sizeof(Item) );
        initializeItem(cell);
        int n = getRandomInt(0, CACHE_SIZE);
        
        int curr = 0;
        while (curr < iter){
            if (NextInLine(fila)->value == n){
                n = getRandomInt(0, CACHE_SIZE);
                curr = -1;
            }            
            curr++;
        }

        cell->value = n;
        insertFila(cell, fila);

        iter++;
    }
    return fila;
}
void showRAMandCache(int mram[], CacheSlot cache[]){
    int rows = 4;
    int higher = MRAM_SIZE/4;
    if (CACHE_SIZE > higher){
        higher = CACHE_SIZE;
    }

    printf("\nCache\t\t\tRAM");
    for (int i = 0; i < higher; i++){
        if (i < CACHE_SIZE){
            printf("\n[ ");
            for (int j = 0; j < BLOCK_SIZE; j++){
                printf("%d ", cache[i].values[j].value);
            }
            printf("]\t\t");
        }
        else{
            printf("\n\t\t\t");
        }

        if (i < MRAM_SIZE/rows){
            for (int k = 0; k < rows; k++){
                int t = (i+(k*MRAM_SIZE/rows));
                if (t < MRAM_SIZE){
                    printf("[%2d] = %d\t", t, mram[t]);
                }
            }
        }

    }
}
void printCacheSlot(CacheSlot* slot){
    printf("[ ");
    for (int j = 0; j < BLOCK_SIZE; j++){
        printf("%d ", slot->values[j].value);
    }
    printf("]");
}
int printCache(CacheSlot cache[]){
    printf("\nCache");
    for (int i = 0; i < CACHE_SIZE; i++){
        printf("\n[ ");
        for (int j = 0; j < BLOCK_SIZE; j++){
            printf("%d ", cache[i].values[j].value);
        }
        printf("]");
    }
    return 1;
}
void initializeCache(CacheSlot cache[]){
    for (int i = 0; i < CACHE_SIZE; i++){
        for (int j = 0; j < BLOCK_SIZE; j++){
            cache[i].values[j].value = NULL_VALUE_FOR_CACHE;
            cache[i].values[j].update = 0;
            cache[i].values[j].RAMaddress = -1;
        }
    }
}
int generateRandomRAMAccess(InputObj testRAM[], int mramsize, int n){
    int i = 0;
    while (i < n){
        testRAM[i].value = getRandomInt(0, mramsize);
        int r;
        r = getRandomInt(0, 10);
        if (r < 2){
            r = 1;
        }
        else{
            r = 0;
        }
        testRAM[i].update = r;
        i++;
    }
    return 1;
}
int mapLRU(int lru[]){
    int iter = 0;
    while (iter < CACHE_SIZE){
        int n = getRandomInt(0, CACHE_SIZE);
        
        int curr = 0;
        while (curr < iter){
            if (lru[curr] == n){
                n = getRandomInt(0, CACHE_SIZE);
                curr = -1;
            }            
            curr++;
        }

        lru[iter] = n;
        iter++;
    }
    return 1;
}
void printLRU(int lru[]){
    printf("\nLRU:\n");
    printf("Proximo Valor -->  [ ");
    for (int i = 0; i < CACHE_SIZE; i++){
        printf("%d ", lru[i]);
    }
    printf("]");
}
CacheSlot* accessRAMSlot(int addr, int update, int ram[], int*pos){

    // static CacheSlot ret;

    CacheSlot* ret = (malloc(sizeof(CacheSlot)));

    int block = addr/BLOCK_SIZE;
    *pos = addr%BLOCK_SIZE;

    // printf("");
    // printf("\nbloco %d, endereco %d", block, addr);

    int b = 0;
    while (b < BLOCK_SIZE){
        ret->values[b].value = ram[(block*BLOCK_SIZE)+b];
        ret->values[b].RAMaddress = (block*BLOCK_SIZE)+b;
        ret->values[b].update = 0;
        b++;
    }
    // printf("");
    if (update){
        printf("\nValor atualizado, de %d para ", ret->values[addr%BLOCK_SIZE].value);
        // printf("\nold value%d", ret->values[addr%BLOCK_SIZE].value);
        ret->values[addr%BLOCK_SIZE].update = 1;
        ret->values[addr%BLOCK_SIZE].value = getRandomInt(100, 999);
        printf("%d", ret->values[addr%BLOCK_SIZE].value);
    }
    return ret;
}
int* generateRandomRAMValues(int mram[], int size, int low, int high){
    int i = 0;
    while(i < size){
        mram[i] = getRandomInt(low, high);
        i++;
    }

    return mram;
}
int needToUpdateBlock(CacheSlot cache[], int pos){
    for (int i = 0; i < BLOCK_SIZE; i++){
        if (cache[pos].values[i].update){
            return 1;
        }
    }
    return 0;
}
void writeBack(CacheSlot cache[], int mram[], int pos){
    // CacheSlot* temp = cache[pos].values;
    for (int i = 0; i < BLOCK_SIZE; i++){
        if (cache[pos].values[i].update){
            // *cache[pos].values[i].RAMaddress = cache[pos].values[i].value;
            printf("\nAtualizando valor %d no index %d na memoria ram ", cache[pos].values[i].value, cache[pos].values[i].RAMaddress);
            mram[cache[pos].values[i].RAMaddress] = cache[pos].values[i].value;
        }
    }
}
void switchCacheBlock(CacheSlot cache[], int mram[], int pos, CacheSlot* block){
    if (needToUpdateBlock(cache, pos)){
        writeBack(cache, mram, pos);
        // writeThrought();
    }
    // printf("\n block values : %d %d", block->values[0].value, block->values[1].value);
    for (int i = 0; i < BLOCK_SIZE; i++){
        cache[pos].values[i].value = block->values[i].value; 
        cache[pos].values[i].update = block->values[i].update; 
        cache[pos].values[i].RAMaddress = block->values[i].RAMaddress; 
    }
}
void copyCacheBlock(CacheSlot cache[], int pos, CacheSlot* block){
    // printf("\n block values : %d %d", block->values[0].value, block->values[1].value);
    // printf("\ncopy%d", pos);
    for (int i = 0; i < BLOCK_SIZE; i++){
        cache[pos].values[i].value = block->values[i].value; 
        cache[pos].values[i].update = block->values[i].update; 
        cache[pos].values[i].RAMaddress = block->values[i].RAMaddress;    
    }
}
int isInsideCache(int search, CacheSlot cache[], int* pos){
    printf("\nProcurando por %d na cache", search);
    for (int i = 0; i < CACHE_SIZE; i++){
        for (int j = 0; j < BLOCK_SIZE; j++){
            if (cache[i].values[j].RAMaddress == search){
                printf("\nValor ja se encontra no bloco %d da cache", i);
                // printf("%d, %d, %d", i, *pos, pos);
                *pos = i;
                // printf("\npasso aq? %d, %d", *pos, i);
                return 1;
            }
        }        
    }
    printf("\nValor nao encontrado na cache");
    return 0;
}
int isCacheFull(CacheSlot cache[]){
    for (int i = 0; i < CACHE_SIZE; i++){
        // for (int j = 0; j < BLOCK_SIZE; j++){
        if (cache[i].values[0].value == NULL_VALUE_FOR_CACHE){
            printf("\nCache nao esta cheia");
            return 0;
            // }
        }        
    }
    printf("\nCache esta cheia");

    return 1;
}
int solveLRU(int lru[], int s){
    int f = 0;
    for (int i = 0; i < CACHE_SIZE; i++){
        if (lru[i] == s){
            f = i;
            // printf("\nfound at %d", f);
        }
    }
    // printf("\nasdasadsadsadsad %d", f);
    for (int j = f; j < CACHE_SIZE-1; j++){
        // printf("\ntrocou %d %d", lru[j], lru[j+1]);
        lru[j] = lru[j+1];
    }
    lru[CACHE_SIZE-1] = s;
    return 1;
}
int getNextFromLRU(int lru[]){
    // printf("\nnext from lru %d", lru[0]);
    solveLRU(lru, lru[0]);
    // printf("%d", lru[CACHE_SIZE-1]);
    return lru[CACHE_SIZE-1];
}
int handleUserInput(int* update){
    
    // arrumar onde o ponteiro para o buffer vai parar apos
    // cada erro do usuario e iteracao do retry
    // esquece ta certinho
    char buffer[16]; 
    char *p;
    int c;
    memset(buffer, 0, sizeof(buffer));

    printf("\n$.. ");
    
    int i, number, retry = 1;

    while (retry){
        i = 0;
        p = buffer;
        
        // printf("\np pointer is  at: %x ", p);

        while((c = getchar()) != '\n' && c != EOF){
            *p++ = (char)c;
            i++;
        }
        *p++ = 0;
        
        // printf("\np pointer finished at: %x ", p);
        

        number = atoi(buffer);
        // verificar se a primeira posicao do buffer se encontra entre 0 e 9 em ascii
        // 0 -> 48    9 -> 57
        if ((buffer[0] < 48 || buffer[0] > 57 ) || (number < 0 || number > MRAM_SIZE-1)){
            retry = 1;
            *update = 0;
            printf("\nIndex invalido ou nenhum index inserido, digite novamente");
            printf("\n$.. ");
        }
        else{
            retry = 0;
        }
    }
    printf("\n%s", buffer);
    *update = 0;
    if (buffer[i-1] == 42){
        *update = 1;
        buffer[i-1] = 0;
    }
    printf("\nbuffer %s   update", buffer, *update);
    // printf("\nfirst char and int form: %c, %d", buffer[0], buffer[0]);
    return number;
}

int TryToAccessAddress(int mram[], int slot, int update, CacheSlot cache[], Fila* FIFO, int lru[]){
    int pos;
    CacheSlot* ptr;

    ptr = accessRAMSlot(slot, update, mram, &pos);
    printf("\nBloco acessado: ");
    printCacheSlot(ptr);

    if (isInsideCache(ptr->values[pos].RAMaddress, cache, &pos)){
        solveLRU(lru, pos);
        return 1;
    }
    
    int new;

    if (isCacheFull(cache)){
        int method;
        printf("\nEscolha um metodo de substituicao");
        printf("\n0 - LRU\n1 - FIFO");

        int t;         
        method = handleUserInput(&t);
        printf("\nmetodo escolhido: %d", method);
        if (method == 1){
            new = NextInLine(FIFO)->value;
            solveLRU(lru, new);
            printf("\nproximo bloco da cache pelo FIFO (%d)", new);
        }
        else if (method == 0){
            new = getNextFromLRU(lru);
            printf("\nproximo bloco da cache pelo LRU (%d)", new);
        }

        switchCacheBlock(cache, mram, new, ptr);
    }
    else{
        new = NextInLine(FIFO)->value;
        printf("\nProximo acesso a cache por mapeamento associativo (%d)", new);
        solveLRU(lru, new);
        copyCacheBlock(cache, new, ptr);
    }
    return 0;

}
void printMRAM(int mram[]){
    for (int i = 0; i < MRAM_SIZE; i++){
        printf("\n[%2d] = %d", i, mram[i]);
    }
}

int StepByStepUser(){
    int input, update = 0, iter = 0;

    srand(time(NULL));
 
    int lru[10];
    mapLRU(lru);

    int mram[MRAM_SIZE];
    generateRandomRAMValues(mram, MRAM_SIZE, 100, 999);

    CacheSlot cache[CACHE_SIZE];
    initializeCache(cache);

    Fila fila;
    initializeFila(&fila);
    randomizeFila(&fila);


    printf("\nOrdem de acesso a memoria cache pela fila");
    printFila(&fila);  
    printLRU(lru);

    showRAMandCache(mram, cache);
    
    
    while (iter < RAM_TEST_ARRAY_SIZE){
        printf("\n------------------------------------------------------------");
        printf("\n\nDigite um index para acessar da memoria RAM");
        input = handleUserInput(&update);
        printf("\nACESSANDO INDEX '%d' DA MRAM, DE VALOR %d", input, mram[input]);

        TryToAccessAddress(mram, input, update, cache, &fila, lru);
        
        printFila(&fila);
        printLRU(lru);
        
        printf("\n");
        showRAMandCache(mram, cache);
        
        printf("\nPressione alguma tecla para prosseguir");
        printf("\n------------------------------------------------------------");
        getchar();
        iter++;
    }
}
int StepByStepAlone(){
    srand(time(NULL));
 
    int lru[10];
    mapLRU(lru);

    InputObj testArray[RAM_TEST_ARRAY_SIZE];
    generateRandomRAMAccess(testArray, MRAM_SIZE, RAM_TEST_ARRAY_SIZE);

    int mram[MRAM_SIZE];
    generateRandomRAMValues(mram, MRAM_SIZE, 100, 999);

    CacheSlot cache[CACHE_SIZE];
    initializeCache(cache);
    

    Fila fila;
    initializeFila(&fila);
    randomizeFila(&fila);


    printf("\nOrdem de acesso a memoria cache pela fila");
    printFila(&fila);  
    printLRU(lru);
 
    showRAMandCache(mram, cache);
    
    getchar();

    int iter = 0;
    while (iter < RAM_TEST_ARRAY_SIZE){
        printf("\n------------------------------------------------------------");
        printf("\nAcessando index '%d' da memoria ram, de valor %d", testArray[iter].value, mram[testArray[iter].value]);
        TryToAccessAddress(mram, testArray[iter].value, testArray[iter].update, cache, &fila, lru);
        
        printFila(&fila);
        printLRU(lru);
        
        showRAMandCache(mram, cache);
        
        printf("\nPressione alguma tecla para prosseguir");
        printf("\n------------------------------------------------------------");
        getchar();
        iter++;
    }
}

void main(int argc, char* argv[]){
    // modo passo a passo com escolhas do usuario
    if (argc == 2 && strcmp(argv[1], "-s") == 0){
        StepByStepUser();
    }
    // modo passo a passo que o usuario so 
    // escolhe o metodo de substituicao
    else if (argc == 2 && strcmp(argv[1], "-i") == 0){
        StepByStepAlone();
    }
    else{
        printf("Argumentos incorretos!\n");
        printf("Modo de uso:\n");
        printf("$ '%s -s' (modo passo a passo)\n", argv[0]);
        printf("$ '%s -i' (modo com acessos aleatorios a memoria)\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}