#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "controle.h"
#include <unistd.h>

controle_t CONTROLE_GLOBAL;

static long now_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000L + tv.tv_usec/1000L;
}

void controle_init(controle_t *c, int M, int N, aeronave_t **aero_list) {
    c->M = M;
    c->N = N;
    c->setores = calloc(M, sizeof(setor_t));
    for (int i = 0; i < M; ++i) setor_init(&c->setores[i], i);
    c->aeronaves = aero_list;
    pthread_mutex_init(&c->mutex, NULL);
    pthread_cond_init(&c->cond, NULL);
    c->simulacao_ativa = 1;
}

void controle_destroy(controle_t *c) {
    for (int i = 0; i < c->M; ++i) setor_destroy(&c->setores[i]);
    free(c->setores);
    pthread_mutex_destroy(&c->mutex);
    pthread_cond_destroy(&c->cond);
}

// internal: concede setores livres para pedidos na fila (maior prioridade primeiro)
static void processar_pedidos(controle_t *c) {
    for (int i = 0; i < c->M; ++i) {
        setor_t *s = &c->setores[i];
        pthread_mutex_lock(&s->mutex);
        if (s->ocupado_por == -1 && s->waitlist != NULL) {
            // pop highest priority
            request_t *r = s->waitlist;
            s->waitlist = r->next;
            s->ocupado_por = r->aeronave_id;
            r->next = NULL;
            aeronave_t *a = r->aeronave_ptr;
            free(r);
            // sinalizar aeronave
            pthread_mutex_lock(&a->mutex);
            a->granted = 1;
            pthread_cond_signal(&a->cond);
            pthread_mutex_unlock(&a->mutex);
        }
        pthread_mutex_unlock(&s->mutex);
    }
}

// thread do controlador
static void *controle_thread(void *arg) {
    controle_t *c = (controle_t*)arg;
    while (1) {
        pthread_mutex_lock(&c->mutex);
        // espera até ser sinalizado (pedido / liberação) ou até timeout curto para reavaliar
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 200 * 1000000; // 200ms
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }
        pthread_cond_timedwait(&c->cond, &c->mutex, &ts);
        pthread_mutex_unlock(&c->mutex);

        // processar pedidos: conceder sempre que possível
        processar_pedidos(c);

        // condição de parada: se todas aeronaves terminaram (checar)
        int all_done = 1;
        for (int i = 0; i < c->N; ++i) {
            if (!c->aeronaves[i]->terminou) { all_done = 0; break; }
        }
        if (all_done) break;
    }
    // liberar recursos
    c->simulacao_ativa = 0;
    return NULL;
}

void controle_start(controle_t *c) {
    pthread_create(&c->thread, NULL, controle_thread, c);
}

void controle_join(controle_t *c) {
    pthread_join(c->thread, NULL);
}

// aeronave solicita setor: cria request e adiciona à fila do setor, depois sinaliza o controlador
// comportamento: aeronave aguarda indefinidamente nessa implementação (o tempo limite e liberação do setor
// atual é feita por estratégia aplicada no lado da aeronave - para simplificar, implementamos timeout por
// tentativa explícita). Aqui apenas enfileiramos e acordamos o controlador
void solicitar_setor(controle_t *c, int setor_id, aeronave_t *a) {
    if (setor_id < 0 || setor_id >= c->M) {
        fprintf(stderr, "Aeronave %d solicitou setor inválido %d\n", a->id, setor_id);
        return;
    }
    request_t *r = malloc(sizeof(request_t));
    r->aeronave_id = a->id;
    r->prioridade = a->prioridade;
    r->aeronave_ptr = a;
    r->next = NULL;
    // inserir ordenado na waitlist do setor
    setor_add_request(&c->setores[setor_id], r);
    // sinalizar o controlador para processar pedidos
    pthread_mutex_lock(&c->mutex);
    pthread_cond_signal(&c->cond);
    pthread_mutex_unlock(&c->mutex);
}

// aeronave libera setor (chamada depois de simular o voo). O controlador será acordado para
// conceder o setor para próximo pedido (se houver)
void liberar_setor(controle_t *c, int setor_id, aeronave_t *a) {
    if (setor_id < 0 || setor_id >= c->M) {
        fprintf(stderr, "Aeronave %d liberou setor inválido %d\n", a->id, setor_id);
        return;
    }
    setor_t *s = &c->setores[setor_id];
    pthread_mutex_lock(&s->mutex);
    if (s->ocupado_por == a->id) {
        s->ocupado_por = -1;
    } else {
        // possível que tenhamos não marcado corretamente: apenas garantir
        s->ocupado_por = -1;
    }
    pthread_mutex_unlock(&s->mutex);

    // sinalizar controlador para reavaliar filas
    pthread_mutex_lock(&c->mutex);
    pthread_cond_signal(&c->cond);
    pthread_mutex_unlock(&c->mutex);
}
