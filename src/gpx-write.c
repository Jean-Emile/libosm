/*
 * gpx-write.c - write GPX file from OSM_Data
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "osm.h"

static int in_node_list(uint64_t *list, int num, uint64_t id) {
    if (num == 0)
        return -1;
    int32_t upper = num - 1,
            lower = 0,
            pos   = -1;
    while (upper >= lower) {
        pos = floor((upper+lower)/2);
        if (list[pos] < id)
            lower = pos + 1;
        else if (list[pos] > id)
            upper = pos - 1;
        else 
            return pos;
    }
    return -1;
}

static int find_node(OSM_Node_List *list, uint64_t id) {
    int upper = list->num - 1,
        lower = 0,
        pos   = -1;
    if (debug) 
        fprintf(stderr, "%s:%d:%s(): upper=%d, id=%lu\n",
                        __FILE__, __LINE__, __FUNCTION__, upper, id);
    while (upper >= lower) {
        pos = floor((upper+lower)/2);
        if (list->data[pos]->id < id)
            lower = pos + 1;
        else if (list->data[pos]->id > id)
            upper = pos - 1;
        else 
            return pos;
    }
    return -1;
}

static int gpx_sort_nodes(const void *a, const void *b) {
    OSM_Node *n = *(OSM_Node * const *)a;
    OSM_Node *m = *(OSM_Node * const *)b;
    if      (n->id > m->id) return  1;
    else if (n->id < m->id) return -1;
    else                    return  0;
}


uint64_t *osm_gpx_write_init(OSM_Data *data, uint32_t *num) {
    int i, k;
    uint32_t num_nodes = 0;
    uint64_t *way_nodes = malloc(sizeof(uint64_t) * data->nodes->num);
    qsort(data->nodes->data, data->nodes->num, sizeof(OSM_Node *), gpx_sort_nodes);
    if (debug) {
        int x;
        for (x=0; x<data->nodes->num; x++) {
            fprintf(stderr, "%s:%d:%s(): node num=% 5d id=%lu\n", 
                __FILE__, __LINE__, __FUNCTION__, x, data->nodes->data[x]->id);
        }
    }

    for (i=0; i<data->ways->num; i++) {
        if (debug) 
            fprintf(stderr, "%s:%d:%s(): way num=% 5d id=%lu\n", 
                __FILE__, __LINE__, __FUNCTION__, i, data->ways->data[i]->id);
        OSM_Way *w = data->ways->data[i]; 
        k = 0;
        while (w->nodes[k]) {
            if (in_node_list(way_nodes, num_nodes, w->nodes[k]) == -1) {
                way_nodes[num_nodes] = w->nodes[k];
                ++num_nodes;
                qsort(way_nodes, num_nodes, sizeof(uint64_t), osm_cmp_member);
            }
            k++;
        }
    }
    *num = num_nodes;
    return way_nodes;
}

void osm_gpx_write_header(char *who, FILE *outfh) {
    fprintf(outfh, "<?xml version='1.0' encoding='UTF-8'?>\n");
    fprintf(outfh, "<gpx version=\"1.1\" generator=\"%s (libosm v" LIBOSM_VERSION  ")\"\n"
                   "     xmlns=\"http://www.topografix.com/GPX/1/1\">\n",
//                    "     xmlns=\"http://www.topografix.com/GPX/1/1\"\n"
//                    "     xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 "
//                                  "http://www.topografix.com/GPX/1/1/gpx.xsd\">\n",
                    who);
    fprintf(outfh, " <metadata>\n"
                   "  <name></name>\n"
                   "  <desc></desc>\n"
                   "  <author>\n"
                   "   <name></name>\n"
                   "   <email domain=\"\" id=\"\"/>\n"
                   "  </author>\n"
                   "  <copyright author=\"\">\n"
                   "  <year></year>\n"
                   "  <license></license>\n"
                   "  </copyright>\n"
                   " </metadata>\n");
}

void osm_gpx_write_footer(FILE *outfh) {
    fprintf(outfh, "</gpx>\n");
}

void osm_gpx_write_tags(OSM_Tag_List *t, FILE *outfh) {
    int i, k;
    char osm_keys[4][32] = { "ele" , "name", "description", "url" };
    char gpx_keys[4][32] = { "ele", "name", "desc", "link" };
    for (k=0; k<4; k++) {
        for (i=0; i<t->num; i++) {
            if (strcmp(t->data[i].key, osm_keys[k]) == 0)
                fprintf(outfh, "  <%s>%s</%s>\n",
                            gpx_keys[k],
                            osm_encode_xml(t->data[i].val),
                            gpx_keys[k]);
        }
    }
}

void osm_gpx_write_node(OSM_Node *n, FILE *outfh, int is_trk) {
    char *type = "wpt";
    char *indent = " ";
    if (is_trk) {
        type = "trkpt";
        indent = "   ";
    }

    fprintf(outfh, "%s<!-- node id=\"%lu\" -->\n", indent, n->id);
    fprintf(outfh, "%s<%s lat=\"%.7f\" lon=\"%.7f\"", indent, type, n->lat, n->lon);
    if (n->tags != NULL && n->tags->num) {
        fprintf(outfh, ">\n");
        osm_gpx_write_tags(n->tags, outfh);
        fprintf(outfh, "%s</%s>\n", indent, type);
    }
    else {
        fprintf(outfh, "/>\n");
    }
}

void osm_gpx_write(OSM_Data *data, FILE *outfh, char *creator) {
    uint32_t num_nodes = 0;
    uint64_t *nodes = osm_gpx_write_init(data, &num_nodes);
    int i, k;
    if (debug)
        fprintf(stderr, "%s:%d:%s(): num_nodes=%u\n", 
                    __FILE__, __LINE__, __FUNCTION__, num_nodes);

    osm_gpx_write_header(creator, outfh);
    for (i=0; i<data->nodes->num; i++) {
        OSM_Node *n = data->nodes->data[i];
        if (in_node_list(nodes, num_nodes, n->id) == -1) {
            osm_gpx_write_node(n, outfh, 0);
            if (debug)
                fprintf(stderr, "%s:%d:%s(): nodes=%lu not used by way\n", 
                        __FILE__, __LINE__, __FUNCTION__, n->id);
        }
        else {
            if (debug)
                fprintf(stderr, "%s:%d:%s(): nodes=%lu used by way\n", 
                        __FILE__, __LINE__, __FUNCTION__, n->id);
        }
    }
    if (data->ways->num) {
        fprintf(outfh, " <trk>\n");
        for (i=0; i<data->ways->num; i++) {
            OSM_Way *w = data->ways->data[i];
            if (debug)
                fprintf(stderr, "%s:%d:%s(): way=%lu\n",
                        __FILE__, __LINE__, __FUNCTION__, w->id);

            fprintf(outfh, "  <!-- way id=\"%lu\" -->\n", w->id);
            fprintf(outfh, "  <trkseg>\n");
            int pos;
            k=0;
            while (w->nodes[k]) {
                pos = find_node(data->nodes, w->nodes[k]);
                if (debug)
                    fprintf(stderr, "%s:%d:%s(): way=%lu, ref=%lu, pos=%d\n",
                            __FILE__, __LINE__, __FUNCTION__, w->id, w->nodes[k], pos);
                if (pos >= 0)
                    osm_gpx_write_node(data->nodes->data[pos], outfh, 1);
                k++;
            }
            fprintf(outfh, "  </trkseg>\n");
        }
        fprintf(outfh, " </trk>\n");
    }
    osm_gpx_write_footer(outfh);
}

/* END */
