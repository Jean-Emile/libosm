/*
 * xml.c - .osm XML parsing, main file
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#define _GNU_SOURCE /* strndup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <time.h>

#include "osm.h"


uint64_t osm_timestamp2epoch(char *timestamp) {
    struct tm tm;
    strptime(timestamp, "%Y-%m-%dT%H:%M:%SZ", &tm);
    time_t ep = mktime(&tm);
    return (uint64_t)ep;    
}

char *osm_xml_decode(char *src) {
    char buffer[LINE_SIZE];
    char *dest = buffer;
    char *source = src;
    while (*src) {
        if (*src == '&') {
            if (strncmp(src, "&amp;", 5) == 0) {
                *dest = '&'; ++dest;
                src += 4;
            }
            else if (strncmp(src, "&quot;", 6) == 0) {
                *dest = '"'; ++dest;
                src += 5;
            }
            else {
                fprintf(stderr, "warning: missing decode for %s\n", src);
            }
        }
        else {
            *dest = *src;
            ++dest;
        }
        ++src;
    }
    *dest = '\0';
    if (debug) 
        fprintf(stderr, "%s:%d:%s: src='%s', dest='%s'\n", 
                        __FILE__, __LINE__, __FUNCTION__, source, buffer);
    
    if (buffer[0])
        return strdup(buffer);
    else
        return "";
}

char *osm_xml_fetch_param(char *src, char *str, char *dest) {
    char *start, *ptr, *res;

    char search[64] = "";
    strncpy(search, str, 62);
    ptr = search;
    while (*ptr) ++ptr;
    *ptr = '=';
    ++ptr;
    *ptr = '\0';

    start = strstr(src, search);
    if (start == NULL) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): no %s in line: %s\n", 
                            __FILE__, __LINE__, __FUNCTION__, search, src);
        return NULL;
    }
    start += strlen(search)+1;

    if (*start && (*start == '"' || *start == '\'')) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): empty value...\n", __FILE__, __LINE__, __FUNCTION__);
        return NULL; /* empty value */
    }

    ptr = start;
    while (*ptr && *(ptr + 1)) {
        if ((*ptr == '"' || *ptr == '\'')
         && (   *(ptr+1) == ' ' || *(ptr+1) == '\t'
             || *(ptr+1) == '/' || *(ptr+1) == '>')) {
            res = strncpy(dest, (const char *)start, ptr - start);
            dest[ptr-start] = '\0';
            if (debug)
                fprintf(stderr, "%s:%d:%s(): param %s\"%s\"\n", 
                            __FILE__, __LINE__, __FUNCTION__, search, dest);
            return dest;
        }
        ++ptr;
    }
    if (debug)
        fprintf(stderr, "%s:%d:%s(): param %s: we should not be here\n", 
                __FILE__, __LINE__, __FUNCTION__, search);
    return NULL;
}

static void find_starts(FILE *file, long int *nodes, long int *ways, long int *relations /*, long int *changesets*/){
    char *buffer;
    char *line;

    buffer = malloc(LINE_SIZE);
    fseek(file, 0, SEEK_SET);
    *nodes = 0;
    *ways  = 0;
    *relations = 0;
    while (1) {
        buffer = fgets(buffer, LINE_SIZE, file);
        if (buffer == NULL) {
            return;
        }
        line = buffer;
        while (*line && (*line == ' ' || *line == '\t'))
            ++line;
        if (!*nodes) {
            if (strncmp(line, "<node ", 6) == 0) {
                *nodes = ftell(file) - strlen(buffer);
                continue;
            }
        }

        if (!*ways) {
            if (strncmp(line, "<way ", 5) == 0) {
                *ways = ftell(file) - strlen(buffer);
                continue;
            }
        }

        if (!*relations) {
            if (strncmp(line, "<relation ", 10) == 0) {
                *relations = ftell(file) - strlen(buffer);
                continue;
            }
        }

        else {
            free(buffer);
            if (debug)
                fprintf(stderr, "%s:%d:%s(): <node>=%lu, <way>=%lu, <relation>=%lu\n", 
                __FILE__, __LINE__, __FUNCTION__, *nodes, *ways, *relations);
            return;
        }
    }
}

OSM_Data *osm_xml_parse(OSM_File *F,
              int mode,
              OSM_BBox *bbox,
              int (*node_filter)(OSM_Node *),
              int (*way_filter)(OSM_Way *),
              int (*rel_filter)(OSM_Relation *)/*,
              int (*cset_filter)(OSM_Changeset *) */
        )
{
    struct osm_members *wanted = NULL;
    long int node_start = 0, way_start = 0, rel_start = 0;
    OSM_Data *data = NULL;

    data = malloc(sizeof(OSM_Data));

    if (mode != OSMDATA_DUMP) {
        wanted = malloc(sizeof(struct osm_members));
        wanted->data = malloc(sizeof(uint64_t) * 1024);
        wanted->num = 0;
        wanted->size = 1024;
    }
    find_starts(F->file, &node_start, &way_start, &rel_start);
    
    data->relations = 
        osm_xml_parse_relations(rel_start, F->file, mode, rel_filter, wanted);

    if (mode == OSMDATA_REL)
        mode = OSMDATA_WAY;
    data->ways = 
        osm_xml_parse_ways(way_start, F->file, mode, way_filter, wanted);

    if (mode == OSMDATA_WAY)
        mode = OSMDATA_NODE;
    data->nodes = 
        osm_xml_parse_nodes(node_start, F->file, mode, node_filter, wanted);

    if (debug)
        fprintf(stderr, "%s:%d:%s(): nodes=%u, ways=%u, relations=%u\n", 
            __FILE__, __LINE__, __FUNCTION__,
            data->nodes     != NULL ? data->nodes->num     : 0,
            data->ways      != NULL ? data->ways->num      : 0,
            data->relations != NULL ? data->relations->num : 0);
    
    if (debug) {
        int x;
        for (x=0; x<data->nodes->num; x++) {
            fprintf(stderr, "%s:%d:%s(): num=% 5d id=%lu\n", 
                __FILE__, __LINE__, __FUNCTION__, x, data->nodes->data[x]->id);
        }
    }
    return data;
}

/* END */
