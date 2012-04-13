/*
 * xml-way.c - .osm XML parsing, way part
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

OSM_Way *osm_xml_get_way(FILE *file, char *buffer, char *param) {
    char *line, *str;
    OSM_Way *W = NULL;
    
    buffer = fgets(buffer, LINE_SIZE, file);
    if (buffer == NULL) {
        if (!feof(file)) {
            perror("error reading .osm file");
        }
        if (debug)
            fprintf(stderr, "%s:%d:%s(): EOF\n",
                    __FILE__, __LINE__, __FUNCTION__);

        return (OSM_Way *)NULL;
    }
    
    line = buffer;
    trim_left(line);
    if (strncmp(line, "<way ", 5) != 0) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): not a <way line: %s\n",
                    __FILE__, __LINE__, __FUNCTION__, line);
        return (OSM_Way *)NULL;
    }

    str = osm_xml_fetch_param(line, "id", param);
    if (str == NULL) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): no ID parameter\n",
                    __FILE__, __LINE__, __FUNCTION__);
        return (OSM_Way *)NULL;
    }

    W = malloc(sizeof(OSM_Way));
    /* 
       setting this to 2048 is a crude hack... 
       if we start with a lower number like 16, realloc'ing 
       will alloc INTO W->tags (or my current gcc on debian sid  
        [gcc version 4.4.5 (Debian 4.4.4-17)] is buggy). 
       This will break if some way has than ~2050 nodes, i.e. when
       you start writing into W->tags...
       ... maybe I should rewrite this to use linked lists, but we're
       doing so many slow malloc()/free()... 
    */
    W->nodes = malloc(sizeof(uint64_t) * 2048);
    W->tags = malloc(sizeof(OSM_Tag_List));
    W->tags->data = malloc(sizeof(OSM_Tag) * 16);
    W->tags->size = 16;
    W->tags->num  = 0;


    int size_nodes = 2048;
    int num_nodes  = 0;
    uint64_t *nodes_ptr = W->nodes;
    W->id = atol(str);
    if (W->id == 0) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): no ID parameter: atol() failed\n",
                    __FILE__, __LINE__, __FUNCTION__);
        free(W->nodes);
        free(W->tags->data);
        free(W->tags);
        free(W);
        return (OSM_Way *)NULL;
    }
    str = osm_xml_fetch_param(line, "user", param);
    if (str == NULL) 
        W->user = "";
    else 
        W->user = strdup(str);

    str = osm_xml_fetch_param(line, "uid", param);
    if (str == NULL) 
        W->uid = 0;
    else 
        W->uid = atoi(str);

    str = osm_xml_fetch_param(line, "version", param);
    if (str == NULL) 
        W->version = 0;
    else 
        W->version = atoi(str);

    str = osm_xml_fetch_param(line, "changeset", param);
    if (str == NULL) 
        W->changeset = 0;
    else 
        W->changeset = atol(str);

    str = osm_xml_fetch_param(line, "timestamp", param);
    if (str == NULL) 
        W->timestamp = 0;
    else 
        W->timestamp = osm_timestamp2epoch(str);

    // FIXME handle <way id="1234"/> => way w/o nodes,tags?
    while (1) {
        buffer = fgets(buffer, LINE_SIZE, file);
        if (buffer == NULL) {
            if (!feof(file))
                perror("error reading .osm file");
            if (debug)
                fprintf(stderr, "%s:%d:%s(): EOF\n",
                        __FILE__, __LINE__, __FUNCTION__);
            osm_free_way(W);
            return (OSM_Way *)NULL;
        }
        
        line = buffer;
        trim_left(line);

        if (strncmp(line, "<nd ", 4) == 0) {
            uint64_t id = 0;
            str = osm_xml_fetch_param(line, "ref", param);
            if (str != NULL) 
                id = atol(str);
            if (id) {
                osm_realloc_nodes(W->nodes, num_nodes, &size_nodes);
                nodes_ptr = W->nodes + num_nodes;
                *nodes_ptr = id;
                ++nodes_ptr;
                *nodes_ptr = 0;
                ++num_nodes;
            }
            else {
                if (debug)
                    fprintf(stderr, "%s:%d:%s(): no ref in <nd\n",
                                __FILE__, __LINE__, __FUNCTION__);
            }
        }
        else if (strncmp(line, "<tag ", 5) == 0) {
            /* osm_realloc_tag_list(W->tags);
            */
            int pos = W->tags->num;
            FETCH_TAG(W->tags, pos);
            /* 
            str = osm_xml_fetch_param(line, "k", param);
            if (str == NULL)
                continue;
            W->tags->data[pos].key = strdup(str);

            str = osm_xml_fetch_param(line, "v", param);
            if (str == NULL)
                W->tags->data[pos].val = "";
            else 
                W->tags->data[pos].val = strdup(str);
            */
            if (debug)
                    fprintf(stderr, "%s:%d:%s(): way=%lu tag: k=%s, v=%s\n",
                                __FILE__, __LINE__, __FUNCTION__,
                                W->id, W->tags->data[pos].key,
                                W->tags->data[pos].val);
            /* W->tags->num += 1; */
        }
        else if (strncmp(line, "</way>", 6) == 0) {
            if (debug)
                    fprintf(stderr, "%s:%d:%s(): way=%lu </way>\n",
                                __FILE__, __LINE__, __FUNCTION__, W->id);
            // W->nodes = realloc(W->nodes, sizeof(uint64_t) * num_nodes + 1); // truncate
            return W;
        }
    }
}
    
OSM_Way_List *osm_xml_parse_ways(long int start, 
                        FILE *file,
                        int mode,
                        int(*filter)(OSM_Way *w),
                        struct osm_members *wanted)
{
    OSM_Way_List *wl = NULL;
    OSM_Way       *W = NULL;
    char *buffer, *param; 

    buffer = malloc(LINE_SIZE);
    param  = malloc(LINE_SIZE);
    wl     = malloc(sizeof(OSM_Way_List));
    wl->data = malloc(sizeof(OSM_Way) * 32);
    wl->size = 32;
    wl->num  = 0;

    fseek(file, start, SEEK_SET);

    for (W = osm_xml_get_way(file, buffer, param);
        W != NULL;
        W = osm_xml_get_way(file, buffer, param)) {
        if (mode == OSMDATA_WAY && filter != NULL) {
            if ((osm_is_member(wanted, W->id) == -1) && !filter(W)) {
                if (debug)
                    fprintf(stderr, "%s:%d:%s(): way=%lu filtered and not a member\n",
                                __FILE__, __LINE__, __FUNCTION__, W->id);
                osm_free_way(W);
                continue;
            }
        }
        else if (mode == OSMDATA_WAY && osm_is_member(wanted, W->id) == -1) {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): way=%lu not a member\n",
                            __FILE__, __LINE__, __FUNCTION__, W->id);
            osm_free_way(W);
            continue;
        }
        else if (filter != NULL && !filter(W)) {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): way=%lu filtered\n",
                            __FILE__, __LINE__, __FUNCTION__, W->id);
            osm_free_way(W);
            continue;
        }

        osm_realloc_way_list(wl);
        wl->data[wl->num] = W;
        wl->num += 1;
        if (mode != OSMDATA_DUMP) {
            int i = 0;
            while (W->nodes[i])
                ++i;
            uint64_t *ref = malloc(sizeof(uint64_t) * i); 
            // uint64_t *ref_p = ref;
            i=0;
            while (W->nodes[i]) {
                ref[i] = W->nodes[i];
                ++i;
            }
            if (debug)
                fprintf(stderr, "%s:%d:%s(): way=%lu adding %d members\n",
                            __FILE__, __LINE__, __FUNCTION__, W->id, i);
            osm_add_members(wanted, i, ref, 1);
        }
    }

    free(buffer);
    free(param);
    if (debug)
        fprintf(stderr, "%s:%d:%s(): returning %d ways\n",
                    __FILE__, __LINE__, __FUNCTION__, wl->num);
    return wl;
}

/* END */
