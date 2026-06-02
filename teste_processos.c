#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

/* simulando cpu bound */
void rodar_cpu_bound(long max_ops) {
    long i;
    volatile double x = 1.0; 
    /* esse volatile foi sugestao do gpt */
    
    for (i = 0; i < max_ops; i++) {
        x = (x + 0.5) * 0.9999;
    }
}

/* simulando io bound */
void rodar_io_bound(long max_ops) {
    long i;
    FILE *fp;
    
    /* eu acho que algo assim eh suficiente, so abrir escrever e fechar */
    for (i = 0; i < max_ops; i++) {
        fp = fopen("/dev/null", "w");
        if (fp != NULL) {
            fprintf(fp, "io");
            fclose(fp);
        }
    }
}

/* forma simples 50/50 cpu e io, da pra mudar, mas assim fica mais facil e direto*/
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <num_processos> <ops_io> <ops_cpu>\n", argv[0]);
        return 1;
    }

    int num_processos = atoi(argv[1]);
    long ops_io = atol(argv[2]);
    long ops_cpu = atol(argv[3]);

    if (num_processos % 2 != 0) {
        fprintf(stderr, "Criar um numero par de processos\n");
        return 1;
    }

    struct timeval t_fork, t_fim;
    pid_t pid;
    int i;

    for (i = 0; i < num_processos; i++) {

        /* tempo de criacao do processo */
        gettimeofday(&t_fork, NULL);

        pid = fork();

        if (pid < 0) {
            perror("Erro no fork");
            exit(1);
        } 
        else if (pid == 0) {
            /* PROCESSO FILHO */
            int meu_tipo_io = (i < num_processos / 2); /* exato 50/50 */



            if (meu_tipo_io) {
                rodar_io_bound(ops_io);
            } else {
                rodar_cpu_bound(ops_cpu);
            }

            /* tempo final */
            gettimeofday(&t_fim, NULL);

            /* tempo de retorno */
            double tempo_retorno = (t_fim.tv_sec - t_fork.tv_sec) +
                                  (t_fim.tv_usec - t_fork.tv_usec) / 1000000.0;

            /* acho que tem tudo que precisa p analise dps */
            printf("Processo %d | Tipo: %s | Tempo de Retorno: %.4f segundos\n", 
                   getpid(), meu_tipo_io ? "IO-bound" : "CPU-bound", tempo_retorno);
            fflush(stdout);
            
            exit(0);
        }
    }

    /* PROCESSO PAI aguarda todos os filhos terminarem para o programa fechar */
    for (i = 0; i < num_processos; i++) {
        wait(NULL);
    }

    return 0;
}