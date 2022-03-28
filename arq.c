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



// retorna um valor aleatorio entre low e high
int getRandomInt(int low, int high){
        return rand() % (high-low) + low;
}

// inicializa um item com valor 0 e next com NULL
Item* initializeItem(Item* i){
    i -> value = 0;
    i -> Next = NULL;
    return i;
}
// inicializa uma fila com tamanho 0, first e last com NULL
Fila* initializeFila(Fila* f){
    f->lenght = 0;
    f->First = NULL;
    f->Last = NULL;
    return f;
}
// inseree um item na fila 
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
// remove o ultimo item da fila
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
// remove o ultimo elemento da fila, adiciona-o de volta e retorna um ponteiro para o item realocado
Item* NextInLine(Fila* fila){
    if (fila->lenght == 0){
        return NULL;
    }
    Item* t = RemoveLast(fila);
    insertFila(t, fila);
    return t;
}
// imprime todos os elementos da fila
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
// retorna uma fila de tamanho CACHE_SIZE com valores aleatorios nao repetidos, para mapear o acesso a memoria cache
Fila* randomizeFila(Fila* fila){
    int iter = 0;
    // enquanto a fila for menor que CACHE_SIZE
    // adiciona valores aleatorios nao repetidos
    while (iter < CACHE_SIZE){
        // aloca uma celula na memoria com valor aleatorio
        Item* cell = malloc( sizeof(Item) );
        initializeItem(cell);
        int n = getRandomInt(0, CACHE_SIZE);
        
        // percore pela fila verificando se o valor ja existe
        // caso exista ele nao eh adicionado e geramos outro valor aleatorio
        int curr = 0;
        while (curr < iter){
            if (NextInLine(fila)->value == n){
                n = getRandomInt(0, CACHE_SIZE);
                curr = -1;
            }            
            curr++;
        }

        // por fim se o valor nao existir, ele eh adicionado
        cell->value = n;
        insertFila(cell, fila);

        iter++;
    }
    return fila;
}
// imprime a memoria ram e cache ao lado, bom para mostrar tudo ao mesmo tempo para o usuario
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
// imprime um slot da memoria cache
void printCacheSlot(CacheSlot* slot){
    printf("[ ");
    for (int j = 0; j < BLOCK_SIZE; j++){
        printf("%d ", slot->values[j].value);
    }
    printf("]");
}
// imprime a memoria cache
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
// inicializa memoria cache com valores nulos
void initializeCache(CacheSlot cache[]){
    for (int i = 0; i < CACHE_SIZE; i++){
        for (int j = 0; j < BLOCK_SIZE; j++){
            cache[i].values[j].value = NULL_VALUE_FOR_CACHE;
            cache[i].values[j].update = 0;
            cache[i].values[j].RAMaddress = -1;
        }
    }
}
// gera um vetor de acesso aleatorio na memoria ram para teste automatico
int generateRandomRAMAccess(InputObj testRAM[], int mramsize, int n){
    int i = 0;
    while (i < n){
        testRAM[i].value = getRandomInt(0, mramsize);
        int r;
        r = getRandomInt(0, 10);
        // se o valor gerado for menor que 2, o acesso eh de atualizacao, assim +- 10% dos valores serao de atualizacao
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
// retorna um vetor de acesso aleatorio na memoria ram para o uso do LRU(least recently used)
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
// imprime a sequencia de acesso do LRU
void printLRU(int lru[]){
    printf("\nLRU:\n");
    printf("Proximo Valor -->  [ ");
    for (int i = 0; i < CACHE_SIZE; i++){
        printf("%d ", lru[i]);
    }
    printf("]");
}
// acessa um slot na memoria ram e retorna o bloco em q o elemento procurado se encontra
CacheSlot* accessRAMSlot(int addr, int update, int ram[], int*pos){
    // addr = endereco da memoria ram
    // update = 1 se for uma atualizacao, 0 se for um acesso
    // ram = vetor de memoria ram
    // pos = posicao do bloco em que o elemento esta

    // aloca um CacheSlot na memoria
    CacheSlot* ret = (malloc(sizeof(CacheSlot)));

    // verifica em que bloco o elemento se encontra
    int block = addr/BLOCK_SIZE;
    // recebe a posicao do elemento acessado dentro do bloco
    *pos = addr%BLOCK_SIZE;

    // copia os valores da ram para 'ret'
    int b = 0;
    while (b < BLOCK_SIZE){
        // valor do elemento acessado
        ret->values[b].value = ram[(block*BLOCK_SIZE)+b];
        // endereco do elemnto acessado
        ret->values[b].RAMaddress = (block*BLOCK_SIZE)+b;
        ret->values[b].update = 0;
        b++;
    }
    // caso precise ser atualizado o valor eh modificado por um nome aleatorio
    // e a variavel update muda para 1 para indicar que o elemento precisa passar
    // pelo write-back
    if (update){
        printf("\nValor atualizado, de %d para ", ret->values[addr%BLOCK_SIZE].value);

        ret->values[addr%BLOCK_SIZE].update = 1;
        ret->values[addr%BLOCK_SIZE].value = getRandomInt(100, 999);
       
        printf("%d", ret->values[addr%BLOCK_SIZE].value);
    }
    return ret;
}
// gera um vetor de valores aleatorios(repetidos ou nao) na memoria ram prontas para acesso
int* generateRandomRAMValues(int mram[], int size, int low, int high){
    int i = 0;
    while(i < size){
        mram[i] = getRandomInt(low, high);
        i++;
    }

    return mram;
}
// checa se o bloco precisa ser atualizado
int needToUpdateBlock(CacheSlot cache[], int pos){
    // cache = vetor de slots da memoria cache
    // pos = posicao do bloco na memoria cache
    for (int i = 0; i < BLOCK_SIZE; i++){
        if (cache[pos].values[i].update){
            return 1;
        }
    }
    return 0;
}
// reescreve os valores atualizados para a memoria ram
void writeBack(CacheSlot cache[], int mram[], int pos){
    for (int i = 0; i < BLOCK_SIZE; i++){
        if (cache[pos].values[i].update){
            printf("\nAtualizando valor %d no index %d na memoria ram ", cache[pos].values[i].value, cache[pos].values[i].RAMaddress);
            
            // acessa o endereco do valor e atualiza o valor dentro da memoria RAM
            mram[cache[pos].values[i].RAMaddress] = cache[pos].values[i].value;
        }
    }
}
// troca CacheBlock's quando a memoria cache ja se encontra cheia

void switchCacheBlock(CacheSlot cache[], int mram[], int pos, CacheSlot* block){
    // pos = posicao do bloco na memoria cache que vai ser substituido
    // block = bloco que vai substituir o bloco na posicao pos

    // caso precise atualizar algum valor no bloco a ser substituido
    // passamos para a funcao writeBack atualizar os valores na memoria RAM
    if (needToUpdateBlock(cache, pos)){
        writeBack(cache, mram, pos);
    }

    // apos atualizar caso necessario, copia os valores do bloco que vai substituir para o bloco que vai substituir
    for (int i = 0; i < BLOCK_SIZE; i++){
        cache[pos].values[i].value = block->values[i].value; 
        cache[pos].values[i].update = block->values[i].update; 
        cache[pos].values[i].RAMaddress = block->values[i].RAMaddress; 
    }
}
// apenas escreve os valores do bloco para a cache, no caso nao precisa atualizar nada
void copyCacheBlock(CacheSlot cache[], int pos, CacheSlot* block){
    for (int i = 0; i < BLOCK_SIZE; i++){
        cache[pos].values[i].value = block->values[i].value; 
        cache[pos].values[i].update = block->values[i].update; 
        cache[pos].values[i].RAMaddress = block->values[i].RAMaddress;    
    }
}
// procura um valor(search) na memoria cache e retorna 1 caso encontre e retorna a sua pos do bloco na cache
int isInsideCache(int search, CacheSlot cache[], int* pos){
    printf("\nProcurando por %d na cache", search);
    for (int i = 0; i < CACHE_SIZE; i++){
        for (int j = 0; j < BLOCK_SIZE; j++){
            if (cache[i].values[j].RAMaddress == search){
                printf("\nValor ja se encontra no bloco %d da cache", i);
                
                *pos = i;
                return 1;
            }
        }        
    }
    printf("\nValor nao encontrado na cache");
    return 0;
}
// retorna 1 se a cache esta cheia
int isCacheFull(CacheSlot cache[]){
    for (int i = 0; i < CACHE_SIZE; i++){
        if (cache[i].values[0].value == NULL_VALUE_FOR_CACHE){
            printf("\nCache nao esta cheia");
            return 0;
        }        
    }
    printf("\nCache esta cheia");
    return 1;
}
// pega a ultima posicao da cache acessada(s) e coloca na ultima posicao do vetor
// demarcando assim, que essa posicao foi a mais recentemente acessada
int solveLRU(int lru[], int s){
    int f = 0;
    // procura o valor 's' dentro do vetor 'lru'
    for (int i = 0; i < CACHE_SIZE; i++){
        if (lru[i] == s){
            f = i;
        }
    }
    
    // passa todos os elementos a frente da posicao do valor 's' um elemento pra tras
    for (int j = f; j < CACHE_SIZE-1; j++){
        lru[j] = lru[j+1];
    }
    // coloca o valor 's' na ultima posicao do vetor
    lru[CACHE_SIZE-1] = s;
    return 1;
}
// retorna o proximo valor que deve ser acessado na memoria cache seguindo o metodo LRU
int getNextFromLRU(int lru[]){
    solveLRU(lru, lru[0]);

    return lru[CACHE_SIZE-1];
}
// cuida do input do usuario para a posicao a ser acessada da memoria RAM
int handleUserInput(int* update){

    // aloca um buffer de 16 char's
    // e um ponteiro que ira apontar para a primeira posicao de 'buffer'    
    char buffer[16]; 
    char *p;
    // 'c' eh usado para capturar o input do usuario
    int c;
    // define todas as posicoes do vetor 'buffer' como '\0'
    memset(buffer, 0, sizeof(buffer));
    printf("\n$.. ");
    // 'i' conta quantos numeros o usuario digitou e 'number' eh o valor que sera retornado
    int i, number, retry = 1;
    // enquanto o usuario nao digitar um numero valido
    while (retry){
        i = 0;
        p = buffer;
        while((c = getchar()) != '\n' && c != EOF){
            // copia o input do usuario para o buffer
            *p++ = (char)c;
            
            i++;
        }
        // coloca o '\0' no final do buffer e transfoma o numero em inteiro        
        *p++ = 0;
        number = atoi(buffer);

        // verifica se a primeira posicao do buffer se encontra entre 0 e 9 em ascii
        // e se o numero é posistivo e menor que o tamanho da memoria RAM
        // 0 -> 48    9 -> 57
        // TODO: verificar todos os indices de buffer
        if ((buffer[0] < 48 || buffer[0] > 57 ) || (number < 0 || number > MRAM_SIZE-1)){
            // caso encontre algum erro ele deixa 'retry' como 0 para que o usuario tente novamente
            retry = 1;
            *update = 0;
            printf("\nIndex invalido ou nenhum index inserido, digite novamente");
            printf("\n$.. ");
        }
        else{
            retry = 0;
        }
    }
    // printf("\n%s", buffer);
    *update = 0;
    // caso a ultima posicao do 'buffer' seja um '*' o endereco acessado deve ser atualizado
    if (buffer[i-1] == 42){
        *update = 1;
        buffer[i-1] = 0;
    }
    printf("\nbuffer %s   update", buffer, *update);
    return number;
}
// cuida de toda a logica de acessar uma poiscao da memoria RAM e se deve ou nao copiar o bloco para a CACHE
int TryToAccessAddress(int mram[], int slot, int update, CacheSlot cache[], Fila* FIFO, int lru[]){
    int pos;
    CacheSlot* ptr;
    // acessa a posicao da memoria RAM
    ptr = accessRAMSlot(slot, update, mram, &pos);
    printf("\nBloco acessado: ");
    printCacheSlot(ptr);

    // caso o endereco ja se encontre na cache retorna 1
    if (isInsideCache(ptr->values[pos].RAMaddress, cache, &pos)){
        solveLRU(lru, pos);
        return 1;
    }
    
    int new;
    // caso a cache esteja cheia pedimos ao usuario o metodo de substituicao
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
        // por fim trocamos os blocos necessarios de acordo com a escolha do usuario
        switchCacheBlock(cache, mram, new, ptr);
    }
    else{
        // caso nao esteja cheia, pegamos a primeira posicao livre da cache pela fila
        new = NextInLine(FIFO)->value;
        printf("\nProximo acesso a cache por mapeamento associativo (%d)", new);
        solveLRU(lru, new);
        copyCacheBlock(cache, new, ptr);
    }
    return 0;

}
// imprime os valores da memoria RAM
void printMRAM(int mram[]){
    for (int i = 0; i < MRAM_SIZE; i++){
        printf("\n[%2d] = %d", i, mram[i]);
    }
}
// funcao onde o usuario anda passo a passo pelo programa
int StepByStepUser(){
    // inicializa o necessario para o programa funcionar
    int input, update = 0, iter = 0;
    srand(time(NULL));
    int lru[CACHE_SIZE];
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
    
    // TODO: fazer o programa rodar em loop ate que o usuario digite 'quit'
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
// funcao onde o usuario passa pelo programa passo a passo porem sem escolher qual posicao da RAM eh acessada
int StepByStepAlone(){
    // incializa o necessario para o programa funcionar
    srand(time(NULL));
    int lru[CACHE_SIZE];
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
    // modo passo a passo que o usuario so escolhe o metodo de substituicao
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