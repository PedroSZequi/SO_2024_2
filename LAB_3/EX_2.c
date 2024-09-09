// Pedro de Souza Zequi - 10419805 - 04p11
// Lab 3 - Exercício 1
#include <stdio.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>  

int main() {
    pid_t pid = fork();  

    if (pid < 0) {
        // Erro na criação do processo
        printf("Erro ao criar o processo\n");
        return 1;
    } 
    else if (pid == 0) {
        // Código do processo filho
        printf("Processo filho (PID: %d)\n", getpid());
        for (int i = 0; i < 5; i++) {
            printf("Mensagem %d do processo filho\n", i + 1);
            sleep(1);  // 1 segundo entre cada mensagem 
        }
    } 
    else {
        // Código do processo pai
        wait(NULL);  // Espera o processo filho terminar
        printf("Processo filho terminou, processo pai continuando\n");
    }

    return 0;
}