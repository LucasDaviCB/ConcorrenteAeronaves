#ifndef SETOR_H
#define SETOR_H

#include <pthread.h>

typedef struct request {
    int aeronave_id;
    unsigned int prioridade;
    struct aeronave_t *aeronave_ptr; // forward
    struct request *next;
} request_t;

typedef struct setor {
    int id;
    int ocupado_por; // -1 se livre, senao id da aeronave
    pthread_mutex_t mutex; // protege o setor e sua fila
    request_t *waitlist; // lista ligada de pedidos (ordenada por prioridade decrescente)
} setor_t;

void setor_init(setor_t *s, int id);
void setor_destroy(setor_t *s);
void setor_add_request(setor_t *s, request_t *r);
void setor_remove_request_by_aeronave(setor_t *s, int aeronave_id);
request_t* setor_pop_highest_priority(setor_t *s);

#endif // SETOR_H
