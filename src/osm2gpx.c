/*
 * osm2gpx.c - convert a .osm XML file to GPX
 *           - example and test for libosm...
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
char *name = "osm2gpx";

int skip_rels(OSM_Relation *r) { return 0; }

int main(int argc, char **argv) {
    if (argc == 1 || argv[1][0] == '-') {
        fprintf(stderr, "%s: Usage: %s file.osm > file.gpx\n",
                        name, name);
        exit(1);
    }
        
    char *file = argv[1];
    
    osm_init();
    
    OSM_File *F = osm_open(file, OSM_FTYPE_XML);
    if (F == NULL)
        return 1;

    OSM_Data *O = osm_xml_parse(F, OSMDATA_DUMP, NULL, NULL, NULL, skip_rels);
    if (O == NULL)
        return 1;

    osm_close(F);
    osm_gpx_write(O, stdout, name);
    return 0;
}

/* END */
