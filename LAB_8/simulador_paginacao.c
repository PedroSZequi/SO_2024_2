// Lab 8 - Paginação
// Pedro de Souza Zequi - 10419805 - Turma 04P11
#include <stdio.h>
#include <stdlib.h>

#define TAMANHO_PAGINA 4096    
#define NUM_FRAMES 10          
#define NUM_PAGINAS 20         

// Estrutura de um Frame na Memória Física
typedef struct {
    int ocupado;               // Indica se o frame está ocupado (1) ou livre (0)
    int id_pagina;             // Página alocada nesse frame, se houver
} Frame;

// Estrutura de uma Página na Memória Virtual
typedef struct {
    int id_frame;              // Frame ao qual a página está mapeada (-1 se não estiver na memória física)
} Pagina;

// Estrutura de uma Tabela de Páginas
typedef struct {
    int paginas[NUM_PAGINAS];  // Mapeia o índice da página para o índice do frame correspondente (-1 se não estiver mapeado)
} TabelaPaginas;

// Estrutura de um Processo
typedef struct {
    int pid;                  // ID do processo
    int *enderecos;           // Array com endereços que o processo vai acessar
    int num_enderecos;        // Quantidade de endereços a serem acessados
    int tamanho_processo;     // Tamanho total do processo em bytes
} Processo;

// Estruturas para memória física, memória virtual e tabela de páginas
Frame memoria_fisica[NUM_FRAMES];
Pagina memoria_virtual[NUM_PAGINAS];
TabelaPaginas tabela_paginas;

// Função para inicializar a Memória Física, Memória Virtual e Tabela de Páginas
void inicializar_memoria() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        memoria_fisica[i].ocupado = 0;
        memoria_fisica[i].id_pagina = -1;
    }
   
    for (int i = 0; i < NUM_PAGINAS; i++) {
        memoria_virtual[i].id_frame = -1;
        tabela_paginas.paginas[i] = -1;
    }
}

// Função para alocar um frame para uma página
int alocar_frame(int id_pagina) {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (!memoria_fisica[i].ocupado) {
            memoria_fisica[i].ocupado = 1;
            memoria_fisica[i].id_pagina = id_pagina;
            tabela_paginas.paginas[id_pagina] = i;
            memoria_virtual[id_pagina].id_frame = i;
            printf("Frame %d alocado para a página %d.\n", i, id_pagina);
            return i;
        }
    }
    printf("Erro: Memória física cheia! Page fault gerado.\n");
    return -1; // Memória cheia
}

// Função para desalocar uma página da memória
void desalocar_pagina(int id_pagina) {
    int id_frame = tabela_paginas.paginas[id_pagina];
    if (id_frame != -1) {
        memoria_fisica[id_frame].ocupado = 0;
        memoria_fisica[id_frame].id_pagina = -1;
        tabela_paginas.paginas[id_pagina] = -1;
        memoria_virtual[id_pagina].id_frame = -1;
        printf("Página %d desalocada do frame %d.\n", id_pagina, id_frame);
    } else {
        printf("A página %d não está alocada.\n", id_pagina);
    }
}

// Função para traduzir um endereço virtual em físico
int traduzir_endereco(int endereco_virtual) {
    if (endereco_virtual < 0 || endereco_virtual >= NUM_PAGINAS * TAMANHO_PAGINA) {
        printf("Erro: Endereço virtual %d fora do intervalo.\n", endereco_virtual);
        return -1;
    }

    int id_pagina = endereco_virtual / TAMANHO_PAGINA;
    int offset = endereco_virtual % TAMANHO_PAGINA;
   
    int id_frame = tabela_paginas.paginas[id_pagina];
    if (id_frame == -1) {
        printf("Page fault na página %d\n", id_pagina);
        return -1; // Indica page fault
    }
   
    int endereco_fisico = (id_frame * TAMANHO_PAGINA) + offset;
    printf("Endereço virtual %d traduzido para endereço físico %d.\n", endereco_virtual, endereco_fisico);
    return endereco_fisico;
}

// Função de simulação 
void simular_processo(Processo processo) {
    printf("Simulando o processo %d:\n", processo.pid);
   
    for (int i = 0; i < processo.num_enderecos; i++) {
        int endereco_virtual = processo.enderecos[i];
       
        // Verifica se a página está na memória física
        int id_pagina = endereco_virtual / TAMANHO_PAGINA;
       
        // Verifica o limite de página
        if (id_pagina < 0 || id_pagina >= NUM_PAGINAS) {
            printf("Erro: Página %d fora do intervalo.\n", id_pagina);
            continue;
        }

        if (tabela_paginas.paginas[id_pagina] == -1) {
            printf("Page fault: Página %d não está na memória física.\n", id_pagina);
            if (alocar_frame(id_pagina) == -1) {
                printf("Erro: Não foi possível alocar a página %d (memória cheia).\n", id_pagina);
                continue;
            }
        }
       
        traduzir_endereco(endereco_virtual);  // Traduzir o endereço virtual para físico
    }
}

int main() {
    inicializar_memoria();
   
    // Criação de um processo de exemplo
    Processo processo;
    processo.pid = 1;
    processo.tamanho_processo = 5 * TAMANHO_PAGINA;
    processo.num_enderecos = 3;
    processo.enderecos = (int *) malloc(processo.num_enderecos * sizeof(int));
   
    if (processo.enderecos == NULL) {
        printf("Erro: Falha na alocação de memória para endereços do processo.\n");
        return 1;
    }

    processo.enderecos[0] = 0 * TAMANHO_PAGINA + 100;  // Endereço na página 0
    processo.enderecos[1] = 3 * TAMANHO_PAGINA + 200;  // Endereço na página 3
    processo.enderecos[2] = 5 * TAMANHO_PAGINA + 50;   // Endereço na página 5
   
    // Executa a simulação do processo
    simular_processo(processo);
   
    // Libera a memória alocada para os endereços do processo
    free(processo.enderecos);
   
    return 0;
}