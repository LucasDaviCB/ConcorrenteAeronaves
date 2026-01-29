#ifndef CONTROLE_H
#define CONTROLE_H

#include "setor.h"
#include "aeronave.h"

typedef struct controle_t {
    int M; // numero de setores
    int N; // numero de aeronaves
    setor_t *setores; // array de M setores
    aeronave_t **aeronaves; // array de N pointers
    pthread_t thread;
    pthread_mutex_t mutex; // protege sinais ao controlador
    pthread_cond_t cond; // sinal para acordar controlador (novos pedidos / liberacoes)
    int simulacao_ativa;
} controle_t;

void controle_init(controle_t *c, int M, int N, aeronave_t **aero_list);
void controle_start(controle_t *c);
void controle_join(controle_t *c);
void controle_destroy(controle_t *c);

// chamadas usadas por aeronaves:
void solicitar_setor(controle_t *c, int setor_id, aeronave_t *a);
void liberar_setor(controle_t *c, int setor_id, aeronave_t *a);

extern controle_t CONTROLE_GLOBAL;

#endif // CONTROLE_H
