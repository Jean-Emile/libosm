/*
 * xml-write.c - .osm XML writing
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#include <stdio.h>

#include "osm.h"

void osm_xml_write_header(char *who, FILE *outfh) {
    fprintf(outfh, "<?xml version='1.0' encoding='UTF-8'?>\n");
    fprintf(outfh, "<osm version=\"0.6\" generator=\"%s (libosm v" LIBOSM_VERSION  ")\">\n",
                    who);
}

void osm_xml_write_footer(FILE *outfh) {
    fprintf(outfh, "</osm>\n");
}

void osm_xml_write_tags(OSM_Tag_List *t, FILE *outfh) {
    int i;
    for (i=0; i<t->num; i++) {
        fprintf(outfh, "  <tag k=\"%s\" v=\"%s\"/>\n",
                            t->data[i].key,
                            osm_encode_xml(t->data[i].val));
    }
}

void osm_xml_write_node(OSM_Node *n, FILE *outfh) {
    char tsbuf[21];
    
    fprintf(outfh, " <node id=\"%li\" lon=\"%.7f\" lat=\"%.7f\"",
            n->id, n->lon, n->lat);

    if (n->version)
        fprintf(outfh, " version=\"%u\"", n->version);
    if (*n->user)
        fprintf(outfh, " user=\"%s\"", n->user);
    if (n->uid)
        fprintf(outfh, " uid=\"%u\"", n->uid);
    if (n->changeset)
        fprintf(outfh, " changeset=\"%lu\"", n->changeset);
    if (n->timestamp) {
        osm_pbf_timestamp(n->timestamp, tsbuf);
        fprintf(outfh, " timestamp=\"%s\"", tsbuf);
    }
    if (n->tags != NULL && n->tags->num) {
        fprintf(outfh, ">\n");
        osm_xml_write_tags(n->tags, outfh);
        fprintf(outfh, " </node>\n");
    }
    else {
        fprintf(outfh, "/>\n");
    }
}

void osm_xml_write_way(OSM_Way *w, FILE *outfh) {
    char tsbuf[21];
    fprintf(outfh, " <way id=\"%li\"", w->id);
    if (w->version)
        fprintf(outfh, " version=\"%u\"", w->version);
    if (*w->user)
        fprintf(outfh, " user=\"%s\"", w->user);
    if (w->uid)
        fprintf(outfh, " uid=\"%u\"", w->uid);
    if (w->changeset)
        fprintf(outfh, " changeset=\"%lu\"", w->changeset);
    if (w->timestamp) {
        osm_pbf_timestamp(w->timestamp, tsbuf);
        fprintf(outfh, " timestamp=\"%s\"", tsbuf);
    }
    if (w->tags == NULL && w->nodes[0] == 0) {
        fprintf(outfh, "/>\n");
    }
    else {
        fprintf(outfh, ">\n");
        int i = 0;
        while (w->nodes[i] != 0) {
            fprintf(outfh, "  <nd ref=\"%li\"/>\n", w->nodes[i]);
            ++i;
        }
        if (w->tags != NULL)
            osm_xml_write_tags(w->tags, outfh);
        fprintf(outfh, " </way>\n");
    }
}

void osm_xml_write_relation(OSM_Relation *r, FILE *outfh) {
    char tsbuf[21];

    fprintf(outfh, " <relation id=\"%li\"", r->id);
    if (r->version)
        fprintf(outfh, " version=\"%u\"", r->version);
    if (*r->user)
        fprintf(outfh, " user=\"%s\"", r->user);
    if (r->uid)
        fprintf(outfh, " uid=\"%u\"", r->uid);
    if (r->changeset)
        fprintf(outfh, " changeset=\"%li\"", r->changeset);

    if (r->timestamp) {
        osm_pbf_timestamp(r->timestamp, tsbuf);
        fprintf(outfh, " timestamp=\"%s\"", tsbuf);
    }
    if (r->tags == NULL && r->member == 0) {
        fprintf(outfh, "/>\n");
    }
    else {
        fprintf(outfh, ">\n");
        int i = 0;
        for (i=0; i<r->member->num; i++) {
            fprintf(outfh, "  <member type=\"%s\" ref=\"%li\" role=\"%s\"/>\n",
                    osm_relmember_type(r->member->data[i].type),
                    r->member->data[i].ref,
                    r->member->data[i].role);
        }
        if (r->tags != NULL)
            osm_xml_write_tags(r->tags, outfh);
        fprintf(outfh, " </relation>\n");
    }
}

/* END */
