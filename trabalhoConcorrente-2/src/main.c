#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "aeronave.h"
#include "controle.h"

extern controle_t CONTROLE_GLOBAL;

static int *gera_rota_aleatoria(int M, int *len_out) {

 // Gera uma rota aleatória composta por índices de setores
 //A rota tem comprimento mínimo 2 e máximo 8 (ou M, se M < 8)

    int maxlen = M < 8 ? M : 8;
    int len = 2 + rand() % (maxlen - 1);
    int *rota = malloc(sizeof(int) * len);
    for (int i = 0; i < len; ++i) {
        rota[i] = rand() % M;
    }
    *len_out = len;
    return rota;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <num_setores M> <num_aeronaves N>\n", argv[0]);
        return 1;
    }
    int M = atoi(argv[1]);
    int N = atoi(argv[2]);
    if (M <= 0 || N <= 0) {
        fprintf(stderr, "M e N devem ser > 0\n");
        return 1;
    }
    srand(time(NULL));

    aeronave_t **aero_list = malloc(sizeof(aeronave_t*) * N);
    for (int i = 0; i < N; ++i) {
        aero_list[i] = malloc(sizeof(aeronave_t));
    }

    // Inicializa o controlador centralizado com M setores e N aeronaves
    // Cria as estruturas internas das filas de prioridade de cada setor
    controle_init(&CONTROLE_GLOBAL, M, N, aero_list);
    // Inicia a thread do controlador
    controle_start(&CONTROLE_GLOBAL);

    // criar aeronaves com prioridades aleatórias 1..1000
    for (int i = 0; i < N; ++i) {
        unsigned int prio = 1 + (rand() % 1000);
        int len;
        int *rota = gera_rota_aleatoria(M, &len);
        aeronave_init(aero_list[i], i, prio, rota, len);
        free(rota);
    }

    // iniciar threads das aeronaves
    for (int i = 0; i < N; ++i) {
        aeronave_start(aero_list[i]);
    }

    // aguardar aeronaves terminarem
    for (int i = 0; i < N; ++i) {
        aeronave_join(aero_list[i]);
    }

    // aguardar controlador terminar
    controle_join(&CONTROLE_GLOBAL);

    // calcular e imprimir tempos médios de espera
    printf("\n=== Estatísticas finais ===\n");
    double soma_medias = 0.0;
    for (int i = 0; i < N; ++i) {
        aeronave_t *a = aero_list[i];
        // Evita divisão por zero caso uma aeronave não tenha feito pedidos
        double media = (a->pedidos_realizados ? (double)a->total_wait_ms / a->pedidos_realizados : 0.0);
        printf("Aeronave %d (prio=%u): pedidos=%d, tempo_medio_espera_ms=%.2f\n",
               a->id, a->prioridade, a->pedidos_realizados, media);
        soma_medias += media;
    }
    printf("Tempo medio de espera (média entre aeronaves) = %.2f ms\n", soma_medias / N);

    // liberar
    controle_destroy(&CONTROLE_GLOBAL);
    for (int i = 0; i < N; ++i) {
        pthread_mutex_destroy(&aero_list[i]->mutex);
        pthread_cond_destroy(&aero_list[i]->cond);
        free(aero_list[i]);
    }
    free(aero_list);
    return 0;
}
