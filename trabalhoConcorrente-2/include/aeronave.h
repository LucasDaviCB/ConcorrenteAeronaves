#ifndef AERONAVE_H
#define AERONAVE_H

#include <pthread.h>

#define MAX_ROTA 128

typedef struct aeronave_t {
    int id;
    unsigned int prioridade;
    int rota_len;
    int rota[MAX_ROTA];
    int pos; // índice atual na rota (setor atual é rota[pos])
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond; // sinal quando controle concede setor
    int granted; // 1 se teve concessão para o setor solicitado
    long total_wait_ms; // soma dos tempos de espera
    int pedidos_realizados;
    int terminou;
} aeronave_t;

void aeronave_init(aeronave_t *a, int id, unsigned int prioridade, int *rota, int rota_len);
void aeronave_start(aeronave_t *a);
void aeronave_join(aeronave_t *a);

#endif // AERONAVE_H
