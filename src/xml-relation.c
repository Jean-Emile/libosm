/*
 * xml-relation.c - .osm XML parsing, relation part
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


OSM_Relation *osm_xml_get_relation(FILE *file, char *buffer, char *param) {
    char *line, *str;
    OSM_Relation *rel = NULL;

    buffer = fgets(buffer, LINE_SIZE, file);
    if (buffer == NULL) {
        if (!feof(file)) {
            perror("error reading .osm file");
        }
        if (debug)
            fprintf(stderr, "%s:%d:%s(): EOF\n", __FILE__, __LINE__, __FUNCTION__);
        return (OSM_Relation *)NULL;
    }

    line = buffer;

    trim_left(line);
    if (strncmp(line, "<relation ", 10) != 0) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): not a <relation line: %s\n", 
                            __FILE__, __LINE__, __FUNCTION__, line);
        return (OSM_Relation *)NULL;
    }

    str = osm_xml_fetch_param(line, "id", param);
    if (str == NULL) {
        if (debug)
            fprintf(stderr, "%s:%d:%s(): no ID for relation\n", 
                            __FILE__, __LINE__, __FUNCTION__);
        return (OSM_Relation *)NULL;
    }

    rel = malloc(sizeof(OSM_Relation));
    rel->id = atol(str);
    if (rel->id == 0) {
        free(rel);
        if (debug)
            fprintf(stderr, "%s:%d:%s(): no ID for relation: atol() failed\n", 
                            __FILE__, __LINE__, __FUNCTION__);
        return (OSM_Relation *)NULL;
    }
    rel->member = malloc(sizeof(OSM_Rel_Member_List));
    rel->member->data = malloc(sizeof(OSM_Rel_Member) * 2048);
    rel->member->size = 2048;
    rel->member->num  = 0;

    rel->tags = malloc(sizeof(OSM_Tag_List));
    rel->tags->data = malloc(sizeof(OSM_Tag) * 64);
    rel->tags->size = 64;
    rel->tags->num  = 0;
    
    str = osm_xml_fetch_param(line, "user", param);
    if (str == NULL)
        rel->user = "";
    else 
        rel->user = strdup(str);

    str = osm_xml_fetch_param(line, "version", param);
    if (str == NULL)
        rel->version = 0;
    else
        rel->version = atoi(str);

    str = osm_xml_fetch_param(line, "changeset", param);
    if (str == NULL)
        rel->changeset = 0;
    else
        rel->changeset = atoi(str);

    str = osm_xml_fetch_param(line, "uid", param);
    if (str == NULL)
        rel->uid = 0;
    else
        rel->uid = atoi(str);
  
    str = osm_xml_fetch_param(line, "timestamp", param);
    if (str == NULL)
        rel->timestamp = 0;
    else
        rel->timestamp = osm_timestamp2epoch(str);

    while (1) {
        buffer = fgets(buffer, LINE_SIZE, file);
        if (buffer == NULL) {
            if (!feof(file)) {
                perror("error reading .osm file");
                free(rel->user);
                free(rel);
                return (OSM_Relation *)NULL;
            }
            if (debug)
                fprintf(stderr, "%s:%d:%s(): EOF\n", 
                                __FILE__, __LINE__, __FUNCTION__);
            break;
        }
        line = buffer;
        trim_left(line);
        if (strncmp(line, "<member ", 8) == 0) {
            osm_realloc_rel_member(rel->member);
            
            int pos = rel->member->num;
            str = osm_xml_fetch_param(line, "ref", param);
            if (str == NULL) 
                continue;
            rel->member->data[pos].ref = atol(str);

            str = osm_xml_fetch_param(line, "type", param);
            if (str == NULL) 
                rel->member->data[pos].type = OSM_REL_MEMBER_TYPE_UNKNOWN;
            else {
                if (strcmp(str, "node") == 0) 
                    rel->member->data[pos].type = OSM_REL_MEMBER_TYPE_NODE;
                else if (strcmp(str, "way") == 0) 
                    rel->member->data[pos].type = OSM_REL_MEMBER_TYPE_WAY;
                else if (strcmp(str, "relation") == 0) 
                    rel->member->data[pos].type = OSM_REL_MEMBER_TYPE_RELATION;
                else 
                    rel->member->data[pos].type = OSM_REL_MEMBER_TYPE_UNKNOWN;
            }
           
            str = osm_xml_fetch_param(line, "role", param);
            if (str == NULL) 
                rel->member->data[pos].role = "";
            else
                rel->member->data[pos].role = strdup(str);

            if (debug)
                fprintf(stderr, "%s:%d:%s(): rel=%lu, ref=%lu, role=%s type=%d\n", 
                                __FILE__, __LINE__, __FUNCTION__, 
                                rel->id, rel->member->data[pos].ref, 
                                rel->member->data[pos].role,
                                rel->member->data[pos].type);

            rel->member->num += 1;
        }
        else if (strncmp(line, "<tag ", 5) == 0) {
            /* osm_realloc_tag_list(rel->tags);
            */
            int pos = rel->tags->num;
            FETCH_TAG(rel->tags, pos);
            /*
            str = osm_xml_fetch_param(line, "k", param);
            if (str == NULL)
                continue;
            rel->tags->data[pos].key = strdup(str);

            str = osm_xml_fetch_param(line, "v", param);
            if (str == NULL)
                rel->tags->data[pos].val = "";
            else
                rel->tags->data[pos].val = strdup(str);
            */
            if (debug)
                fprintf(stderr, "%s:%d:%s(): rel=%lu, tag: k=%s, v=%s\n", 
                                __FILE__, __LINE__, __FUNCTION__, 
                                rel->id, rel->tags->data[pos].key, 
                                rel->tags->data[pos].val);
            /* rel->tags->num += 1; */
        }
        else if (strncmp(line, "</relation>", 11) == 0) {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): rel=%lu, </relation>\n", 
                                __FILE__, __LINE__, __FUNCTION__, rel->id); 
            return rel;
        }
    }
    /* if we ever get here... */
    // if (debug)
        fprintf(stderr, "%s:%d:%s(): we should not be here ...\n",
                        __FILE__, __LINE__, __FUNCTION__); 
    return (OSM_Relation *)NULL;
}

OSM_Relation_List *osm_xml_parse_relations(long int start, 
                            FILE *file, 
                            int mode, 
                            int(*filter)(OSM_Relation *r),
                            struct osm_members *wanted)
{
    OSM_Relation_List *rl = NULL;
    OSM_Relation      *R  = NULL;
    char *buffer, *param;

    buffer = malloc(LINE_SIZE);
    param  = malloc(LINE_SIZE);
    rl     = malloc(sizeof(OSM_Relation_List));
    rl->data = malloc(sizeof(OSM_Relation) * 2048);
    rl->size = 2048;
    rl->num  = 0;

    fseek(file, start, SEEK_SET);
    
    for (R = osm_xml_get_relation(file, buffer, param);
         R != NULL;
         R = osm_xml_get_relation(file, buffer, param)) 
    {
        if (filter != NULL && !filter(R)) {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): rel=%lu filtered\n",
                                __FILE__, __LINE__, __FUNCTION__, R->id); 
            osm_free_relation(R);
            continue;
        }

        osm_realloc_rel_list(rl);
        rl->data[rl->num] = R;
        rl->num += 1;
        if (mode != OSMDATA_DUMP && R->member->num) {
            int i = 0;
            uint64_t *ref = malloc(sizeof(uint64_t) * R->member->num);
            for (i=0; i<R->member->num; i++) {
                ref[i] = R->member->data[i].ref;
            }
            if (debug)
                fprintf(stderr, "%s:%d:%s(): rel=%lu adding %d members\n",
                                __FILE__, __LINE__, __FUNCTION__, R->id, i); 
            osm_add_members(wanted, i, ref, 1);
        }
        R = osm_xml_get_relation(file, buffer, param);
    }
    free(buffer);
    free(param);
    if (debug)
        fprintf(stderr, "%s:%d:%s(): returning %d relations\n",
                        __FILE__, __LINE__, __FUNCTION__, rl->num); 
    return rl;
}

