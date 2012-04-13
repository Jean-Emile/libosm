/*
 * osmpbf2osm.c - convert .osm.pbf to .osm XML 
 *              - example and test for libosm
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "osm.h"
int debug = 0;

int node2xml(OSM_Node *n) {
    osm_xml_write_node(n, stdout);
    return 0;
}

int way2xml(OSM_Way *w) {
    osm_xml_write_way(w, stdout);
    return 0;
}

int rel2xml(OSM_Relation *r) {
    osm_xml_write_relation(r, stdout);
    return 0;
}

char *name = "osmpbf2osm";

int main(int argc, char **argv) {
    if (argc == 1 || argv[1][0] == '-') {
        fprintf(stderr, "%s: Usage: %s file.osm.pbf > file.osm\n",
                        name, name);
        exit(1);
    }
        
    char *file = argv[1];
    
    osm_init();
    
    OSM_File *F = osm_open(file, OSM_FTYPE_PBF);
    if (F == NULL)
        return 1;
    osm_xml_write_header(name, stdout);
    osm_pbf_parse(F, OSMDATA_DUMP, NULL, node2xml, way2xml, rel2xml);
    osm_xml_write_footer(stdout);
    osm_close(F);
    return 0;
}

/* END */
