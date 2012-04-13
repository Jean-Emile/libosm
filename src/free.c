/*
 * free.c - free allocated structs
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */
#include <stdlib.h>
#include <string.h>

#include "osm.h"

void osm_free_tags(OSM_Tag_List *t) {
    int i;
    if (t == NULL)
        return;

    for (i=0; i< t->num; i++) {
        if (*t->data[i].key)
            free(t->data[i].key);
        if (*t->data[i].val)
            free(t->data[i].val);
    }
    free(t->data);
    free(t);
}

void osm_free_node(OSM_Node *n) {
    if (n == NULL)
        return;
    osm_free_tags(n->tags);
    if (*n->user)
        free(n->user);
    free(n);
}

void osm_free_way(OSM_Way *w) {
    if (w == NULL)
        return;
    osm_free_tags(w->tags);
    if (*w->user)
        free(w->user);
    free(w->nodes);
    free(w);
}

void osm_free_way_list(OSM_Way_List *w) {
    int i = 0;
    for (i=0; i<w->num; i++)
        osm_free_way(w->data[i]);
    free(w);
}

void osm_free_relation(OSM_Relation *r) {
    int i;
    if (r == NULL)
        return;
    osm_free_tags(r->tags);
    if (*r->user)
        free(r->user);
    if (r->member != NULL) {
        for (i=0; i<r->member->num; i++) {
            if (*r->member->data[i].role)
                free(r->member->data[i].role);
        }
        free(r->member->data);
        free(r->member);
    }
    free(r);
}

/* END */
