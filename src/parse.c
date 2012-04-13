/*
 * parse.c - wrapper for osm_(xml|pbf)_parse()
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#include <stdio.h>

#include "osm.h"

OSM_Data *osm_parse(OSM_File *F,
              int mode,
              OSM_BBox *bbox,
              int (*node_filter)(OSM_Node *),
              int (*way_filter)(OSM_Way *),
              int (*rel_filter)(OSM_Relation *)/*,
              int (*cset_filter)(OSM_Changeset *) */
        )
{
    if (mode & OSMDATA_DUMP)
        mode = OSMDATA_DUMP;
    else if (mode & OSMDATA_REL)
        mode = OSMDATA_REL;
    else if (mode & OSMDATA_WAY)
        mode = OSMDATA_WAY;
    else if (mode & OSMDATA_NODE)
        mode = OSMDATA_NODE;
    else if (mode & OSMDATA_BBOX)
        mode = OSMDATA_BBOX;

    if (F->type == OSM_FTYPE_PBF)
        return osm_pbf_parse(F, mode, bbox, node_filter, way_filter, rel_filter);
    else if (F->type == OSM_FTYPE_XML)
        return osm_xml_parse(F, mode, bbox, node_filter, way_filter, rel_filter);

    fprintf(stderr, "cannot parse unknown file type\n");
    return (OSM_Data *)NULL;
}

/* END */
