/*
 * realloc.c - re-allocate more space for the arrays in the structs
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#define _GNU_SOURCE /* strndup ... */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "osm.h"

void osm_realloc_rel_member(OSM_Rel_Member_List *r) {
    if ((float)r->num/(float)r->size > LIST_THRESHOLD) {
        r->size *= 2;
        r->data  = realloc(r->data, sizeof(OSM_Rel_Member) * r->size);
    }
}

void osm_realloc_way_list(OSM_Way_List *w) {
    if ((float)w->num/(float)w->size > LIST_THRESHOLD) {
        w->size *= 2;
        w->data  = realloc(w->data, sizeof(OSM_Way) * w->size);
    }
}

void osm_realloc_tag_list(OSM_Tag_List *t) {
    if ((float)t->num/(float)t->size > LIST_THRESHOLD) {
        t->size *= 2;
        t->data  = realloc(t->data, sizeof(OSM_Tag) * t->size);
    }
}

void osm_realloc_rel_list(OSM_Relation_List *r) {
    if ((float)r->num/(float)r->size > LIST_THRESHOLD) {
        r->size *= 2;
        r->data  = realloc(r->data, sizeof(OSM_Relation) * r->size);
    }
}

void osm_realloc_node_list(OSM_Node_List *n) {
    if ((float)n->num/(float)n->size > LIST_THRESHOLD) {
        n->size *= 2;
        n->data  = realloc(n->data, sizeof(OSM_Node) * n->size);
    }
}

void osm_realloc_nodes(uint64_t *n, int num, int *size) {
    int s = *size;
    if ((float)num/(float)s > LIST_THRESHOLD) {
        *size *= 2;
        n = realloc(n, sizeof(uint64_t) * s);
    }
}

