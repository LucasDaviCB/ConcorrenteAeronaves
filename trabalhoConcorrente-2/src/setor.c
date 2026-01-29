#include <stdlib.h>
#include <string.h>
#include "setor.h"
#include <stdio.h>


// Inicializa um setor do espaço aéreo
// Cada setor começa desocupado (ocupado_por = -1) e com uma lista de espera vazia
// O mutex interno protege tanto o campo ocupado_por quanto a lista waitlist


void setor_init(setor_t *s, int id) {
    s->id = id;
    s->ocupado_por = -1;
    pthread_mutex_init(&s->mutex, NULL);
    s->waitlist = NULL;
}

void setor_destroy(setor_t *s) {
    pthread_mutex_destroy(&s->mutex);
    // liberar waitlist se houver
    request_t *r = s->waitlist;
    while (r) {
        request_t *nx = r->next;
        free(r);
        r = nx;
    }
}

 // Adiciona uma requisição à lista de espera
 //A lista é ordenada por prioridade decrescente

void setor_add_request(setor_t *s, request_t *r) {
    // insere ordenado por prioridade decrescente
    pthread_mutex_lock(&s->mutex);
    request_t **cur = &s->waitlist;
    while (*cur && (*cur)->prioridade >= r->prioridade) {
        cur = &(*cur)->next;
    }
    r->next = *cur;
    *cur = r;
    pthread_mutex_unlock(&s->mutex);
}

void setor_remove_request_by_aeronave(setor_t *s, int aeronave_id) {
    pthread_mutex_lock(&s->mutex);
    request_t **cur = &s->waitlist;
    while (*cur) {
        if ((*cur)->aeronave_id == aeronave_id) {
            request_t *tofree = *cur;
            *cur = tofree->next;
            free(tofree);
            break;
        }
        cur = &(*cur)->next;
    }
    pthread_mutex_unlock(&s->mutex);
}

//Retorna e remove da lista o pedido de maior prioridade

request_t* setor_pop_highest_priority(setor_t *s) {
    pthread_mutex_lock(&s->mutex);
    request_t *r = s->waitlist;
    if (!r) {
        pthread_mutex_unlock(&s->mutex);
        return NULL;
    }
    s->waitlist = r->next;
    r->next = NULL;
    pthread_mutex_unlock(&s->mutex);
    return r;
}
