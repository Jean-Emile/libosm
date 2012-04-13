/*
 * util.c - various helper functions
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE
#endif
#include <time.h>

#include "osm.h"

void osm_init() {
    setenv("TZ", "UTC", 1);    
    tzset();
}

char *osm_relmember_type(int id) {
    switch (id) {
        case 0:
        default:
            return "unknown";
            break;
        case 1:
            return "node";
            break;
        case 2:
            return "way";
            break;
        case 3:
            return "relation";
            break;
    }
}

char *osm_encode_xml(char *src) {
    char buffer[LINE_SIZE];
    char *dest = buffer;
    char *source = src;
    memset(dest, '\0', LINE_SIZE);
    while (*source) {
        switch (*source) {
            case '&':
                *dest = '&'; dest++;
                *dest = 'a'; dest++;
                *dest = 'm'; dest++;
                *dest = 'p'; dest++;
                *dest = ';'; dest++;
                break;
            case '"': 
                *dest = '&'; dest++;
                *dest = 'q'; dest++;
                *dest = 'u'; dest++;
                *dest = 'o'; dest++;
                *dest = 't'; dest++;
                *dest = ';'; dest++;
                break;
            case '<':
                *dest = '&'; dest++;
                *dest = 'l'; dest++;
                *dest = 't'; dest++;
                *dest = ';'; dest++;
                break;
            case '>':
                *dest = '&'; dest++;
                *dest = 'g'; dest++;
                *dest = 't'; dest++;
                *dest = ';'; dest++;
                break;
            default:
                *dest = *source; dest++;
                break;
        }
        source++;     
    }
    *dest = '\0';
    if (debug)
        fprintf(stderr, "%s:%d:%s(): SRC='%s', DEST='%s'\n", 
                         __FILE__, __LINE__, __FUNCTION__, src, buffer);
    dest = buffer;
    return dest;
}

int osm_cmp_member(const void *a, const void *b) {
    if (*(uint64_t *)a > *(uint64_t *)b)
        return 1;
    else if (*(uint64_t *)a < *(uint64_t*)b)
        return -1;
    else
        return 0;
}

void osm_sort_member(struct osm_members *m) {
    qsort(m->data, m->num, sizeof(uint64_t), osm_cmp_member);
}

void osm_add_members(struct osm_members *m, uint32_t num, uint64_t *list, int sort) {
    uint32_t i = 0;
    while ((float)(m->num + num)/(float)m->size > LIST_THRESHOLD) {
        m->size *= 2;
        m->data  = realloc(m->data, sizeof(uint64_t) * m->size);
    }
    uint64_t *ptr = m->data;
    ptr += m->num;
    for (i=0; i<num; i++) {
        *ptr = list[i];
        ++ptr;
        m->num += 1;
    }
    if (sort) 
        qsort(m->data, m->num, sizeof(uint64_t), osm_cmp_member);
    free(list);
}


int osm_is_member(struct osm_members *m, uint64_t id) {
    if (m->num == 0)
        return -1;

    int32_t upper = m->num - 1,
             lower = 0;
    int32_t pos;
    while (upper >= lower) {
        pos = floor((upper+lower)/2);
        if (m->data[pos] < id)
            lower = pos + 1;
        else if (m->data[pos] > id)
            upper = pos - 1;
        else
            return pos;
    }
    return -1;
}

/* END */
