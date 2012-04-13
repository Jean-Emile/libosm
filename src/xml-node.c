/*
 * xml-node.c - .osm XML parsing, node part
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

OSM_Node *osm_xml_get_node(FILE *file, char *buffer, char *param) {
    OSM_Node *N = NULL;
    char *line, *str;

    buffer = fgets(buffer, LINE_SIZE, file);
    if (buffer == NULL) {
        if (!feof(file))
            perror("error reading .osm file");
        else if (debug)
                fprintf(stderr, "%s:%d:%s(): EOF\n",
                            __FILE__, __LINE__, __FUNCTION__);

        return (OSM_Node *)NULL;
    }

    line = buffer;
    trim_left(line);

    if (strncmp(line, "<node ", 5) != 0) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): not a <node line: %s\n",
                        __FILE__, __LINE__, __FUNCTION__, line);
        return (OSM_Node *)NULL;
    }

    str = osm_xml_fetch_param(line, "id", param);
    if (str == NULL) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): no node id\n",
                        __FILE__, __LINE__, __FUNCTION__);
        return (OSM_Node *)NULL;
    }

    N = malloc(sizeof(OSM_Node));
    N->tags = NULL;
    N->id   = atol(str);
    if (N->id == 0) {
        free(N);
        if (debug)
            fprintf(stderr, "%s:%d:%s(): no node id: atol() failed\n",
                        __FILE__, __LINE__, __FUNCTION__);
        return (OSM_Node *)NULL;
    }

    str = osm_xml_fetch_param(line, "lon", param);
    if (str == NULL) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): node=%lu: no node 'lon='\n",
                        __FILE__, __LINE__, __FUNCTION__, N->id);
        free(N);
        return (OSM_Node *)NULL;
    }
    N->lon = atof(str); // FIXME parsing errors?

    str = osm_xml_fetch_param(line, "lat", param);
    if (str == NULL) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): node=%lu: no node 'lat='\n",
                        __FILE__, __LINE__, __FUNCTION__, N->id);
        free(N);
        return (OSM_Node *)NULL;
    }
    N->lat = atof(str); // FIXME parsing errors?

    N->tags = malloc(sizeof(OSM_Tag_List));
    N->tags->data = malloc(sizeof(OSM_Tag) * 16);
    N->tags->size = 16;
    N->tags->num  = 0;

    str = osm_xml_fetch_param(line, "user", param);
    if (str == NULL) 
        N->user = strdup("");
    else
        N->user = strdup(str);

    str = osm_xml_fetch_param(line, "uid", param);
    if (str == NULL) 
        N->uid = 0;
    else
        N->uid = atoi(str);

    str = osm_xml_fetch_param(line, "version", param);
    if (str == NULL) 
        N->version = 0;
    else
        N->version = atoi(str);
    
    str = osm_xml_fetch_param(line, "changeset", param);
    if (str == NULL) 
        N->changeset = 0;
    else
        N->changeset = atol(str);
    
    str = osm_xml_fetch_param(line, "timestamp", param);
    if (str == NULL) 
        N->timestamp = 0;
    else
        N->timestamp = osm_timestamp2epoch(str);

    str = line + strlen(line);
    while (*str != '>') {
        // *str = '\0';
        --str;
    }
    --str;
    if (*str == '/') {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): node=%lu: no tags...\n",
                        __FILE__, __LINE__, __FUNCTION__, N->id);
        return N;
    }

    while (1) {
        buffer = fgets(buffer, LINE_SIZE, file);
        if (buffer == NULL) {
            if (!feof(file))
                perror("error reading .osm file");
            else 
                if (debug)
                    fprintf(stderr, "%s:%d:%s(): node=%lu: EOF\n",
                                __FILE__, __LINE__, __FUNCTION__, N->id);
            osm_free_node(N); 
            return (OSM_Node *)NULL;
        }
        line = buffer;
        trim_left(line);

        if (strncmp(line, "<tag ", 5) == 0) {
            /* osm_realloc_tag_list(N->tags);
            */
            int pos = N->tags->num;
            FETCH_TAG(N->tags, pos);
            /*
            str = osm_xml_fetch_param(line, "k", param);
            if (str == NULL)
                continue;
            N->tags->data[pos].key = strdup(str);

            str = osm_xml_fetch_param(line, "v", param);
            if (str == NULL)
                N->tags->data[pos].val = "";
            else
                N->tags->data[pos].val = strdup(str);
            N->tags->num += 1;
            */
            if (debug)
                fprintf(stderr, "%s:%d:%s(): node=%lu: tag: k=%s, v=%s\n",
                                __FILE__, __LINE__, __FUNCTION__, N->id,
                                N->tags->data[pos].key, N->tags->data[pos].val);
        }
        else if (strncmp(line, "</node>", 6) == 0) {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): node=%lu: </node>\n",
                                __FILE__, __LINE__, __FUNCTION__, N->id);
            return N;
        }
    }
}

OSM_Node_List *osm_xml_parse_nodes(long int start,
                                    FILE *file,
                                    int mode,
                                    int(*filter)(OSM_Node *n),
                                    struct osm_members *wanted)
{
    OSM_Node_List *nl = NULL;
    OSM_Node       *N = NULL;
    char *buffer, *param;

    buffer = malloc(LINE_SIZE);
    param  = malloc(LINE_SIZE);
    nl     = malloc(sizeof(OSM_Node_List));
    nl->data = malloc(sizeof(OSM_Node) * 32);
    nl->size = 32;
    nl->num  = 0;

    fseek(file, start, SEEK_SET);

//    N = osm_xml_get_node(file, buffer, param);
    for (N = osm_xml_get_node(file, buffer, param); N != NULL; N = osm_xml_get_node(file, buffer, param)) {
        if (mode == OSMDATA_NODE && filter != NULL) {
            if ((osm_is_member(wanted, N->id) == -1) && !filter(N)) {
                if (debug)
                    fprintf(stderr, "%s:%d:%s(): node=%lu: not a member and filtered\n",
                                __FILE__, __LINE__, __FUNCTION__, N->id);
                osm_free_node(N);
                continue; 
            }
        }
        else if (mode == OSMDATA_NODE && osm_is_member(wanted, N->id) == -1) {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): node=%lu: not a member\n",
                            __FILE__, __LINE__, __FUNCTION__, N->id);
            osm_free_node(N);
            continue; 
        }
        else if (filter != NULL && !filter(N)) {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): node=%lu: filtered\n",
                            __FILE__, __LINE__, __FUNCTION__, N->id);
            osm_free_node(N);
            continue; 
        }
        osm_realloc_node_list(nl);
        nl->data[nl->num] = N;
        nl->num += 1;
    }

    free(buffer);
    free(param);
    if (debug)
        fprintf(stderr, "%s:%d:%s(): returning %d nodes\n",
                    __FILE__, __LINE__, __FUNCTION__, nl->num);
    return nl;
}

/* END */
