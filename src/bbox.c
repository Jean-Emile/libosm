/*
 * bbox.c - bounding box 
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

OSM_BBox *osm_bbox_from_nodes(OSM_Node_List *n) {
    uint32_t i;
    OSM_BBox *box = malloc(sizeof(OSM_BBox));

    box->top_lat    =  -90.0;
    box->bottom_lat =   90.0;
    box->left_lon   =  180.0;
    box->right_lon  = -180.0;
    for (i=0; i< n->num; i++) {
        if (n->data[i]->lon < box->left_lon)
            box->left_lon = n->data[i]->lon;

        if (n->data[i]->lon > box->right_lon)
            box->right_lon = n->data[i]->lon;

        if (n->data[i]->lat < box->bottom_lat)
            box->bottom_lat = n->data[i]->lat;

        if (n->data[i]->lat > box->top_lat)
            box->top_lat = n->data[i]->lat;
    }

    return box;
}

/* END */
