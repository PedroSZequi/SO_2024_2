// PROJETO 2 - SISTEMAS OPERACIONAIS - SIMULADOR DE PAGINAÇÃO
// Felipe Ujvari Gasparino de Sousa - 10418415 Turma 04P11
// Pedro de Souza Zequi - 10419805 - Turma 04P11
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define MAX_PROCESSOS 10 // Define o número máximo de processos suportados pelo simulador

// Estrutura que representa uma entrada na tabela de páginas
typedef struct {
    int numero_pagina;    // Número da página
    int numero_quadro;    // Quadro correspondente na memória física
    int carregada;        // Indica se a página está carregada na memória (1 para sim, 0 para não)
    int ultimo_acesso;    
} EntradaTabelaPaginas;

// Estrutura que representa a tabela de páginas de um processo
typedef struct {
    EntradaTabelaPaginas *paginas; // Array de entradas de páginas
    int total_paginas;             // Total de páginas no processo
    int *fila_quadros;             // Fila de quadros para implementar substituição FIFO
    int inicio_fila;               // Índice do início da fila
    int fim_fila;                  // Índice do fim da fila
    int total_quadros;             // Total de quadros na memória
} TabelaPaginas;

// Variáveis globais para simulação
int tamanho_memoria, tamanho_pagina, total_quadros, tempo_delay, faltas_pagina = 0, acessos_totais = 0;
char caminho_log[50], politica_substituicao[10];
TabelaPaginas tabela_paginas;
int *memoria_fisica;
FILE *arquivo_log;
int logs_habilitados = 1, tempo_atual = 0;

// Função para validar as configurações carregadas do arquivo
void validar_configuracoes() {
    if (tamanho_memoria % tamanho_pagina != 0) {
        fprintf(stderr, "Erro: TAMANHO_MEMORIA deve ser múltiplo de TAMANHO_PAGINA.\n");
        exit(1);
    }
    if (strcmp(politica_substituicao, "FIFO") != 0 &&
        //strcmp(politica_substituicao, "LRU") != 0 &&
        strcmp(politica_substituicao, "ALEATORIA") != 0) {
        fprintf(stderr, "Erro: POLITICA_SUBSTITUICAO deve ser FIFO, LRU ou ALEATORIA.\n");
        fprintf(stderr, "Valor recebido: '%s'\n", politica_substituicao);
        exit(1);
    }
}

// Função para carregar as configurações do arquivo
void carregar_configuracoes(const char *caminho_config) {
    FILE *arquivo_config = fopen(caminho_config, "r");
    if (!arquivo_config) {
        perror("Erro ao abrir o arquivo de configuração.");
        exit(1);
    }
    // Lê as configurações do arquivo
    fscanf(arquivo_config, "TAMANHO_MEMORIA=%d\n", &tamanho_memoria);
    fscanf(arquivo_config, "TAMANHO_PAGINA=%d\n", &tamanho_pagina);
    fscanf(arquivo_config, "TOTAL_PAGINAS=%d\n", &tabela_paginas.total_paginas);
    fscanf(arquivo_config, "DELAY=%d\n", &tempo_delay);
    fscanf(arquivo_config, "CAMINHO_LOG=%s\n", caminho_log);
    fscanf(arquivo_config, "POLITICA_SUBSTITUICAO=%s\n", politica_substituicao);
    fclose(arquivo_config);

    printf("Configurações carregadas:\n");
    printf("- Tamanho da memória: %d bytes\n", tamanho_memoria);
    printf("- Tamanho das páginas: %d bytes\n", tamanho_pagina);
    printf("- Total de páginas: %d\n", tabela_paginas.total_paginas);
    printf("- Política de substituição: %s\n", politica_substituicao);
    printf("- Delay (segundos): %d\n", tempo_delay);

    validar_configuracoes(); // Valida as configurações antes de continuar

    total_quadros = tamanho_memoria / tamanho_pagina;
    // Alocação dinâmica das estruturas
    tabela_paginas.paginas = malloc(tabela_paginas.total_paginas * sizeof(EntradaTabelaPaginas));
    tabela_paginas.fila_quadros = malloc(total_quadros * sizeof(int));
    memoria_fisica = malloc(tamanho_memoria * sizeof(int));
}

// Função para configurar o arquivo de log
void configurar_arquivo_log() {
    arquivo_log = fopen(caminho_log, "a");
    if (!arquivo_log) {
        perror("Erro ao abrir o arquivo de log.");
        exit(1);
    }
    // Marca o início da simulação no log
    time_t agora = time(NULL);
    fprintf(arquivo_log, "Início da simulação: %s\n", ctime(&agora));
}

// Função para registrar eventos no log
void registrar_evento(const char *evento) {
    if (logs_habilitados) {
        time_t agora = time(NULL);
        fprintf(arquivo_log, "[%s] %s\n", ctime(&agora), evento);
        fflush(arquivo_log);
    }
    printf("LOG: %s\n", evento); // Também exibe no console
}

// Função para simular o atraso no acesso à memória
void simular_delay() {
    sleep(tempo_delay);
}

// Função para carregar uma página na memória, utilizando a política de substituição configurada
void carregar_pagina(int numero_pagina) {
    int quadro_substituido;

    if (strcmp(politica_substituicao, "FIFO") == 0) { // Implementação FIFO
        if (tabela_paginas.fim_fila < total_quadros) {
            quadro_substituido = tabela_paginas.fim_fila++;
        } else {
            quadro_substituido = tabela_paginas.fila_quadros[tabela_paginas.inicio_fila];
            tabela_paginas.inicio_fila = (tabela_paginas.inicio_fila + 1) % total_quadros;
        }
    } else if (strcmp(politica_substituicao, "ALEATORIA") == 0) { // Implementação ALEATÓRIA
        quadro_substituido = rand() % total_quadros;
    } else {
        fprintf(stderr, "Política de substituição não implementada.\n");
        exit(1);
    }

    // Atualiza a tabela de páginas
    tabela_paginas.fila_quadros[quadro_substituido] = numero_pagina;
    tabela_paginas.paginas[numero_pagina].numero_quadro = quadro_substituido;
    tabela_paginas.paginas[numero_pagina].carregada = 1;
    tabela_paginas.paginas[numero_pagina].ultimo_acesso = tempo_atual++;

    // Log do carregamento da página
    char mensagem_log[100];
    snprintf(mensagem_log, sizeof(mensagem_log), "FALTA_PAGINA: Página %d carregada no quadro %d", numero_pagina, quadro_substituido);
    registrar_evento(mensagem_log);

    simular_delay();
    faltas_pagina++;
}

// Função para traduzir endereço lógico para endereço físico
int traduzir_endereco(int endereco_logico) {
    int numero_pagina = endereco_logico / tamanho_pagina; // Determina a página com base no endereço lógico
    int deslocamento = endereco_logico % tamanho_pagina;  // Calcula o deslocamento dentro da página

    // Verifica se a página está carregada na memória física
    if (!tabela_paginas.paginas[numero_pagina].carregada) {
        carregar_pagina(numero_pagina); // Se não, carrega a página na memória
    }

    // Atualiza o tempo do último acesso à página para políticas como LRU
    tabela_paginas.paginas[numero_pagina].ultimo_acesso = tempo_atual++;
    acessos_totais++; // Incrementa o contador de acessos totais

    // Calcula o endereço físico correspondente
    int endereco_fisico = tabela_paginas.paginas[numero_pagina].numero_quadro * tamanho_pagina + deslocamento;
    printf("Endereço lógico %d traduzido para endereço físico %d\n", endereco_logico, endereco_fisico);

    return endereco_fisico; // Retorna o endereço físico resultante
}

// Função para inicializar a tabela de páginas
void inicializar_tabela_paginas() {
    for (int i = 0; i < tabela_paginas.total_paginas; i++) {
        tabela_paginas.paginas[i].numero_pagina = i;      // Define o número da página
        tabela_paginas.paginas[i].numero_quadro = -1;     // Inicializa o quadro como inválido (-1)
        tabela_paginas.paginas[i].carregada = 0;          // Marca a página como não carregada
        tabela_paginas.paginas[i].ultimo_acesso = 0;      // Inicializa o último acesso como 0
    }
    // Inicializa os índices da fila de quadros (para FIFO)
    tabela_paginas.inicio_fila = 0;
    tabela_paginas.fim_fila = 0;
}

// Função principal
int main(int argc, char *argv[]) {
    // Verifica se o arquivo de configuração foi fornecido como argumento
    if (argc < 2) {
        printf("Uso: %s <arquivo de configuração>\n", argv[0]);
        return 1;
    }

    // Carrega as configurações e inicializa os recursos necessários
    carregar_configuracoes(argv[1]);
    configurar_arquivo_log();
    inicializar_tabela_paginas();

    // Apresenta as instruções iniciais ao usuário
    printf("Simulação iniciada. Use comandos:\n- acessar <endereco_logico>\n- relatorio\n- sair\n");

    while (1) {
        char comando[20]; // Armazena o comando digitado pelo usuário
        printf("\nDigite um comando: ");
        scanf("%s", comando); // Lê o comando

        // Comando para acessar um endereço lógico
        if (strcmp(comando, "acessar") == 0) {
            int endereco_logico;
            scanf("%d", &endereco_logico); // Lê o endereço lógico
            traduzir_endereco(endereco_logico); // Traduz o endereço e processa a página
        } 
        // Comando para exibir o relatório de desempenho
        else if (strcmp(comando, "relatorio") == 0) {
            printf("Relatório: Faltas de Página: %d, Acessos Totais: %d\n", faltas_pagina, acessos_totais);
        } 
        // Comando para encerrar a simulação
        else if (strcmp(comando, "sair") == 0) {
            break; // Sai do loop principal
        } 
        // Comando desconhecido
        else {
            printf("Comando desconhecido.\n");
        }
    }

    // Finaliza o programa liberando os recursos alocados
    fclose(arquivo_log); // Fecha o arquivo de log
    free(tabela_paginas.paginas); // Libera a memória da tabela de páginas
    free(tabela_paginas.fila_quadros); // Libera a memória da fila de quadros
    free(memoria_fisica); // Libera a memória física

    return 0; // Retorna com sucesso
}