// Pedro de Souza Zequi - 10419805 - 04p11
// Lab 3 - Exercício 3
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
        // Substitui o processo filho por um novo programa usando exec
        execlp("ls", "ls", "-l", NULL);  // Executa ls -l
        printf("Erro ao executar o exec\n");  // Caso falhe
    } 
    else {
        // Código do processo pai
        wait(NULL);  // Espera o processo filho terminar
        printf("Processo filho terminou, processo pai continuando\n");
    }

    return 0;
}
