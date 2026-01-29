#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "aeronave.h"
#include "controle.h"
#include <string.h>

extern controle_t CONTROLE_GLOBAL;

static long now_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000L + tv.tv_usec/1000L;
}

void aeronave_init(aeronave_t *a, int id, unsigned int prioridade, int *rota, int rota_len) {
    a->id = id;
    a->prioridade = prioridade;
    a->rota_len = rota_len;
    a->pos = 0;
    if (rota_len > MAX_ROTA) rota_len = MAX_ROTA;
    memcpy(a->rota, rota, rota_len * sizeof(int));
    pthread_mutex_init(&a->mutex, NULL);
    pthread_cond_init(&a->cond, NULL);
    a->granted = 0;
    a->total_wait_ms = 0;
    a->pedidos_realizados = 0;
    a->terminou = 0;
}

static void ms_sleep(int ms) {
    usleep(ms * 1000);
}

static void *rotina(void *arg) {
    aeronave_t *a = (aeronave_t*)arg;
    unsigned int seed = time(NULL) ^ (a->id * 7919);
    // a aeronave inicialmente ocupa o primeiro setor da rota (se existir)
    if (a->rota_len == 0) {
        a->terminou = 1;
        return NULL;
    }

    // Ao iniciar, assume que aeronave "está" no primeiro setor da rota (rota[0])
    int setor_atual = a->rota[0];
    // marcar setor inicial como ocupado - pedir ao controle para "pegar" sem aguardar outra
    // (para simplificar, fazemos uma solicitação especial para marcar ocupação inicial)
    // Aqui notificamos o controlador via função de solicitar_setor; ela vai se comportar normalmente.
    // Porém para não bloquear, setamos pos = 0 e já consideramos que está no setor 0
    // Para preservar invariantes do enunciado, vamos considerar que aeronaves começam fora e
    // precisam adquirir o primeiro setor antes de voar
    a->pos = 0;

    while (a->pos < a->rota_len) {
        int destino = a->rota[a->pos];
        long t0 = now_ms();
        // solicita o setor destino
        solicitar_setor(&CONTROLE_GLOBAL, destino, a);

        // espera até ser concedido (com timeout implementado no controlador por timedwait externo)
        pthread_mutex_lock(&a->mutex);
        while (!a->granted) {
            // espera indefinidamente; a concessão é feita pelo controlador via cond signal
            pthread_cond_wait(&a->cond, &a->mutex);
        }
        a->granted = 0;
        pthread_mutex_unlock(&a->mutex);

        long t1 = now_ms();
        long waited = t1 - t0;
        a->total_wait_ms += waited;
        a->pedidos_realizados++;

        // agora "entra" no setor (simula voo)
        // imprimir log
        printf("[%ld] Aeronave %d (prio=%u) entrou no setor %d (rota pos %d/%d). Esperou %ld ms\n",
               t1, a->id, a->prioridade, destino, a->pos+1, a->rota_len, waited);

        // simula tempo de voo dentro do setor: 200ms a 800ms aleatório
        int voo_ms = 200 + (rand_r(&seed) % 601);
        ms_sleep(voo_ms);

        // terminar esse setor: avança posição (libera setor atual)
        liberar_setor(&CONTROLE_GLOBAL, destino, a);
        a->pos++;

        // pequena pausa entre setores
        ms_sleep(50 + (rand_r(&seed) % 151));
    }

    a->terminou = 1;
    printf("[%ld] Aeronave %d terminou sua rota. Tempo medio de espera (ms): %.2f (pedidos=%d)\n",
           now_ms(), a->id, (a->pedidos_realizados? (double)a->total_wait_ms/a->pedidos_realizados : 0.0), a->pedidos_realizados);

    return NULL;
}

void aeronave_start(aeronave_t *a) {
    pthread_create(&a->thread, NULL, rotina, a);
}

void aeronave_join(aeronave_t *a) {
    pthread_join(a->thread, NULL);
}
