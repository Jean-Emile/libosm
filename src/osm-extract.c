/*
 * osm-extract.c - extract data from .osm/.osm.pbf files
 *               - example and test for libosm
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

/* ToDo: usage():
   "b:dr:w:n:u:t:v:PXG
   -b llon,botlat,rlon,toplat - use bounding box instead of full file
   -d  - debug
   -r ID - get relation ID
   -w ID - get way ID
   -n ID - get node ID
   -u USER - fetch objects from user USER
   -v TAG [-v VAL] - only objects with tag TAG (and value VAL)
   -P - file is pbf format
   -X - file is xml format
   -G - write GPX instead of .osm XML
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "osm.h"

#define OSMX_VERSION "0.2"

uint64_t wanted_id = 0;
int debug = 0;
int type;
char *user = NULL, *tag = NULL, *value = NULL;
char *file;
int use_rel = 0, use_way = 0, use_node = 0;
int file_type = OSM_FTYPE_UNKNOWN;
int write_gpx = 0;
OSM_BBox *bbox = NULL;

int rel_wanted(OSM_Relation *r) {
    if (r->id == wanted_id)
        return 1;
    return 0;
}

int way_wanted(OSM_Way *w) {
    if (w->id == wanted_id)
        return 1;
    return 0;
}

int node_wanted(OSM_Node *n) {
    if (n->id == wanted_id)
        return 1;
    return 0;
}

int user_node(OSM_Node *n) {
    if (!*n->user)
        return 0;
    if (strcmp(user, n->user) == 0) 
        return 1;
    return 0;
}

int user_way(OSM_Way *n) {
    if (!*n->user)
        return 0;
    if (strcmp(user, n->user) == 0) 
        return 1;
    return 0;
}
int user_rel(OSM_Relation *n) {
    if (!*n->user)
        return 0;
    if (strcmp(user, n->user) == 0) 
        return 1;
    return 0;
}

int tag_node(OSM_Node *n) {
    int i;
    if (n->tags == NULL)
        return 0;
    if (tag == NULL)
        return 1;
    if (value != NULL) {
        for (i=0; i<n->tags->num; i++) {
            char *key = n->tags->data[i].key;
            char *val = n->tags->data[i].val;
            if (*key && *val)
            {
                if (strcmp(tag, key) == 0 && strcmp(value, val) == 0)
                    return 1;
            }
        }
    }
    else {
        for (i=0; i<n->tags->num; i++) {
            if (*n->tags->data[i].key) {
                char *key = n->tags->data[i].key;
                if (strcmp(tag, key) == 0)
                    return 1;
            }
        }
    }
    return 0;
}

int tag_way(OSM_Way *n) {
    int i;
    if (n->tags == NULL)
        return 0;
    if (tag == NULL)
        return 1;
    if (value != NULL) {
        for (i=0; i<n->tags->num; i++) {
            char *key = n->tags->data[i].key;
            char *val = n->tags->data[i].val;
            if (*key && *val)
            {
                if (strcmp(tag, key) == 0 && strcmp(value, val) == 0)
                    return 1;
            }
        }
    }
    else {
        for (i=0; i<n->tags->num; i++) {
            if (*n->tags->data[i].key) {
                char *key = n->tags->data[i].key;
                if (strcmp(tag, key) == 0)
                    return 1;
            }
        }
    }
    return 0;
}

int tag_rel(OSM_Relation *n) {
    int i;
    if (n->tags == NULL)
        return 0;
    if (tag == NULL)
        return 1;
    if (value != NULL) {
        for (i=0; i<n->tags->num; i++) {
            char *key = n->tags->data[i].key;
            char *val = n->tags->data[i].val;
            if (*key && *val)
            {
                if (strcmp(tag, key) == 0 && strcmp(value, val) == 0)
                    return 1;
            }
        }
    }
    else {
        for (i=0; i<n->tags->num; i++) {
            if (*n->tags->data[i].key) {
                char *key = n->tags->data[i].key;
                if (strcmp(tag, key) == 0)
                    return 1;
            }
        }
    }
    return 0;
}

void parse_args(int argc, char **argv) {
    char c;
    opterr = 0;
    while ((c = getopt(argc, argv, "b:dr:w:n:u:t:v:PXG")) != -1) {
        switch (c) {
            case 'b':
                bbox = malloc(sizeof(OSM_BBox));
                char *str = optarg, *end;
                double box[4];
                int i;
                for (i=0; i<3; i++) {
                    end = strchr(str, ',');
                    if (end == NULL) {
                        fprintf(stderr, "not enough bbox params\n");
                        exit(1);
                    }
                    *end = '\0';
                    box[i] = atof(str);
                    str = end + 1;
                }
                if (!*str) {
                    fprintf(stderr, "not enough bbox params\n");
                    exit(1);
                }
                bbox->left_lon   = box[0];
                bbox->bottom_lat = box[1];
                bbox->right_lon  = box[2];
                bbox->top_lat    = atof(str);
                if (debug) {
                    fprintf(stderr, "BBOX: llon=%.7f blat=%.7f rlon=%.7f tlat=%.7f\n", bbox->left_lon, bbox->bottom_lat, bbox->right_lon,  bbox->top_lat);
                }
                break;

            case 'd':
                debug = 1;
                break;
            case 'r':
                wanted_id = atol(optarg);
                use_rel   = 1;
                break;
            case 'w':
                wanted_id = atol(optarg);
                use_way   = 1;
                break;
            case 'n':
                wanted_id = atol(optarg);
                use_node  = 1;
                break;
            case 'u':
                user = strdup(optarg);
                break;
            case 't':
                tag  = strdup(optarg);
                break;
            case 'v':
                value = strdup(optarg);
                break;
            case 'P':
                file_type = OSM_FTYPE_PBF;
                break;
            case 'X':
                file_type = OSM_FTYPE_XML;
                break;
            case 'G':
                write_gpx = 1;
                break;
            default:
                fprintf(stderr, "unknown option %c\n", c);
                exit(1);
        }
    }
    if (argc == optind) {
        fprintf(stderr, "missing file\n");
        exit(1);
    }
    file = argv[optind];
}

int ftype_by_suffix(char *filename) {
    char *suffix;
    int len = strlen(filename);
    if (len < 5)
        return OSM_FTYPE_UNKNOWN;
    suffix = filename + len - 4;
    if (strcmp(suffix, ".osm") == 0)
        return OSM_FTYPE_XML;
    else if (strcmp(suffix, ".pbf") == 0)
        return OSM_FTYPE_PBF;
    else
        return OSM_FTYPE_UNKNOWN;
}

int main(int argc, char **argv) {
    int i;
    OSM_File *F;
    OSM_Data *O;

    parse_args(argc, argv);

    if (file_type == OSM_FTYPE_UNKNOWN)
        file_type = ftype_by_suffix(file);

    osm_init();

    F = osm_open(file, file_type);
    if (F == NULL)
        return 1;

    if (bbox != NULL) {
        if (tag != NULL) 
            O = osm_parse(F, OSMDATA_BBOX, bbox, tag_node, tag_way, tag_rel);
        else if (user != NULL)
            O = osm_parse(F, OSMDATA_BBOX, bbox, user_node, user_way, user_rel);
        else
            O = osm_parse(F, OSMDATA_BBOX, bbox, NULL, NULL, NULL);
    }
    else if (use_rel)
        O = osm_parse(F, OSMDATA_REL, NULL, NULL, NULL, rel_wanted);
    else if (use_way) 
        O = osm_parse(F, OSMDATA_WAY, NULL, NULL, way_wanted, NULL);
    else if (use_node) 
        O = osm_parse(F, OSMDATA_NODE, NULL, node_wanted, NULL, NULL);
    else if (user != NULL) 
        O = osm_parse(F, OSMDATA_REL, NULL, user_node, user_way, user_rel);
    else if (tag != NULL)
        O = osm_parse(F, OSMDATA_REL, NULL, tag_node, tag_way, tag_rel);    
    else {
        fprintf(stderr, "no selection specified\n");
        exit(1);
    }
    osm_close(F);

    if (write_gpx)
        osm_gpx_write(O, stdout, "osm-extract v" OSMX_VERSION);
    else {
        osm_xml_write_header("osm-extract v" OSMX_VERSION, stdout);
        for (i=0; i<O->nodes->num; i++)
            osm_xml_write_node(O->nodes->data[i], stdout);
        for (i=0; i<O->ways->num; i++)
            osm_xml_write_way(O->ways->data[i], stdout);
        for (i=0; i<O->relations->num; i++)
            osm_xml_write_relation(O->relations->data[i], stdout);
       osm_xml_write_footer(stdout);
    } 
    return 0;
}
