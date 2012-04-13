/*
 * nodes.c - sort OSM_Node_List and find nodes
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
#include <math.h>

#include "osm.h"


int osm_node_pos(OSM_Node_List *n, uint64_t id) {
    int upper = n->num - 1,
        lower = 0,
        pos   = -1,
        ret   = -1;
    while (upper >= lower) {
        pos = floor((upper+lower)/2);
        if (n->data[pos]->id < id)
            lower = pos + 1;
        else if (n->data[pos]->id > id)
            upper = pos - 1;
        else {
            ret = pos;
            break;
        } 
    }
    if (debug)
        fprintf(stderr, "%s:%d:%s(): id=%lu is at pos %d\n",
                        __FILE__, __LINE__, __FUNCTION__, id, ret);
    return ret;
}

int osm_node_cmp(const void *a, const void *b) {
    OSM_Node *A = *(OSM_Node * const *)a;
    OSM_Node *B = *(OSM_Node * const *)b;
    if      (A->id > B->id) return  1;
    else if (A->id < B->id) return -1;
    else                    return  0;
}

void osm_node_list_sort(OSM_Node_List *n) {
    qsort(n->data, n->num, sizeof(OSM_Node *), osm_node_cmp);
}

/* END */
