// PROJETO 1 - SISTEMAS OPERACIONAIS
// Pedro de Souza Zequi - 10419805 - Turma 04P11
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define INITIAL_BALANCE 1000
#define NUM_THREADS 10
#define TRANSFER_AMOUNT 50

// Estrutura para armazenar informações da conta
typedef struct {
    int balance;
    pthread_mutex_t mutex; // Mutex para sincronização
} BankAccount;

// Função para depositar uma quantia na conta
void deposit(BankAccount *account, int amount) {
    // Bloqueia o mutex antes de modificar o saldo -> Mudança feita em relação ao código inicial do enunciado
    pthread_mutex_lock(&account->mutex);
   
    account->balance += amount;
    printf("Depositado: %d, Saldo Atual: %d\n", amount, account->balance);
   
    // Libera o mutex após modificar o saldo -> Mudança feita em relação ao código inicial do enunciado
    pthread_mutex_unlock(&account->mutex);
}

// Função para sacar uma quantia da conta
void withdraw(BankAccount *account, int amount) {
    // Bloqueia o mutex antes de modificar o saldo -> Mudança feita em relação ao código inicial do enunciado
    pthread_mutex_lock(&account->mutex);

    if (account->balance >= amount) {
        account->balance -= amount;
        printf("Sacado: %d, Saldo Atual: %d\n", amount, account->balance);
    } else {
        printf("Saldo Insuficiente para saque: %d\n", amount);
    }

    // Libera o mutex após modificar o saldo -> Mudança feita em relação ao código inicial do enunciado
    pthread_mutex_unlock(&account->mutex);
}

// Função executada pelas threads para depósito
void* transaction_dep(void* arg) {
    BankAccount *account = (BankAccount*)arg;

    // Realiza uma série de depósitos
    for (int i = 0; i < 5; i++) {
        deposit(account, TRANSFER_AMOUNT);  // Depósito de 50 unidades
        sleep(3);  // Simula tempo de processamento
    }

    return NULL;
}

// Função executada pelas threads para saque
void* transaction_sac(void* arg) {
    BankAccount *account = (BankAccount*)arg;

    // Realiza uma série de saques
    for (int i = 0; i < 5; i++) {
        withdraw(account, TRANSFER_AMOUNT);  // Saque de 50 unidades
        sleep(1);  // Simula tempo de processamento
    }

    return NULL;
}

int main() {
    pthread_t threads_dep[NUM_THREADS];  // 10 threads para depósito
    pthread_t threads_sac[NUM_THREADS];  // 10 threads para saque
    BankAccount account;
    account.balance = INITIAL_BALANCE;  // Saldo inicial de 1000 unidades
    pthread_mutex_init(&account.mutex, NULL);  // Inicializa o mutex

    // Cria threads de depósito
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads_dep[i], NULL, transaction_dep, &account) != 0) {
            perror("Falha ao criar thread de depósito");
            exit(EXIT_FAILURE);
        }
    }

    // Cria threads de saque
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads_sac[i], NULL, transaction_sac, &account) != 0) {
            perror("Falha ao criar thread de saque");
            exit(EXIT_FAILURE);
        }
    }

    // Espera as threads de depósito terminarem
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads_dep[i], NULL) != 0) {
            perror("Falha ao esperar thread de depósito");
            exit(EXIT_FAILURE);
        }
    }

    // Espera as threads de saque terminarem
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads_sac[i], NULL) != 0) {
            perror("Falha ao esperar thread de saque");
            exit(EXIT_FAILURE);
        }
    }

    // Destrói o mutex
    pthread_mutex_destroy(&account.mutex);

    // Imprime o saldo final
    printf("Saldo Final: %d\n", account.balance);

    return 0;
}

// Referência consultada: https://www.ibm.com/docs/pt-br/aix/7.3?topic=programming-using-mutexes