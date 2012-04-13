/*
 * waydupes.c - find duplicate ways
 *            - example and test for libosm
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <time.h>

#include "osm.h"

int skip_relations(OSM_Relation *r) {
    return 0;
}

int skip_nodes(OSM_Node *n) {
    return 0;
}

int use_highways(OSM_Way *w) {
    int i;
    if (w->tags == NULL) return 0;
    for (i=0; i<w->tags->num; i++) {
        char *highway = w->tags->data[i].key;
        if (*highway && strcmp(highway, "highway") == 0)
                return 1;
    }
    return 0;
}

#define bbox_area(b) ((b->right_lon - b->left_lon) \
                        * (b->top_lat - b->bottom_lat))

double auto_bbox(double area, long int num_ways, long int num_nodes) {
    /*
        number of computations done (roughly):

                                                        num_ways   num_ways   1
      num_bbox * log(num_nodes) * num_ways + num_bbox * -------- * -------- * - 
                                                        num_bbox   num_bbox   2

      area                               num_ways^2 * size
    = ---- * log(num_nodes) * num_ways + -------------------
      size                                  area * 2


    = 2 area^2 * log(num_nodes) / num_ways + size^2

      area: bbox area of the full .osm file
      size: bbox area of a "tile" 
    */
    // fprintf(stderr, "area=%f\nways=%ld\nnode=%ld\n", area, num_ways, num_nodes);
    return sqrt( sqrt(2 * area * area * log(num_nodes) / num_ways) );
}

void write_gpx(OSM_Data *D, char *gpx_file) {
    int i, k, pos;
    FILE *outfh;

    outfh = fopen(gpx_file, "w");
    if (outfh == NULL) {
        fprintf(stderr, "failed to open GPX file '%s': %s\n", 
                        gpx_file, strerror(errno));
        exit(1);
    }

    osm_gpx_write_header("waydupes", outfh);
    if (D->ways->num) {
        for (i=0; i<D->ways->num; i++) {
            OSM_Way *w = D->ways->data[i];
            k=0;
            fprintf(outfh, " <trk>\n");
            fprintf(outfh, "  <trkseg>\n");
            while (w->nodes[k]) {
                pos = osm_node_pos(D->nodes, w->nodes[k]);
                if (pos >= 0)
                    osm_gpx_write_node(D->nodes->data[pos], outfh, 1);
                k++;
            }
            fprintf(outfh, "  </trkseg>\n");
            fprintf(outfh, " </trk>\n");
        }
    }
    osm_gpx_write_footer(outfh);
    fflush(outfh);
    fclose(outfh);
}

int file_type;
char *file;
char *gpx_file = NULL;
double bbsize = 0.0;
int debug = 0;
int by_location = 0;


void parse_args(int argc, char **argv) {
    char c;
    //opterr = 0;
    while ((c = getopt(argc, argv, "bdlPXg:")) != -1) {
        switch (c) {
            case 'b':
                bbsize = atof(optarg);
                break;
            case 'd':
                debug = 1;
                break;
            case 'l':
                by_location = 1;
                break;
            case 'P':
                file_type = OSM_FTYPE_PBF;
                break;
            case 'X':
                file_type = OSM_FTYPE_XML;
                break;
            case 'g':
                gpx_file = strdup(optarg);
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



int main(int argc, char **argv) {
    int i;
    OSM_File *F;
    OSM_Data *O;
    OSM_Way_List *ways, *dupes;
    OSM_Node_List *G;
    time_t start = time(NULL);

  
    parse_args(argc, argv);

    osm_init();
    F = osm_open(file, file_type);
    if (F == NULL)
        return 1;

    O = osm_parse(F, OSMDATA_WAY, NULL, skip_nodes, use_highways, NULL);
    osm_close(F);
    fprintf(stderr, "parsing file done after %d\n", (int)(time(NULL)-start));
    

    ways = malloc(sizeof(OSM_Way_List));
    ways->data = malloc(sizeof(OSM_Way) * 65536);
    ways->num  = 0;
    ways->size = 65536;

    dupes = malloc(sizeof(OSM_Way_List));
    dupes->data = malloc(sizeof(OSM_Way) * 65536);
    dupes->num  = 0;
    dupes->size = 65536;
 
    G = malloc(sizeof(OSM_Node_List));

    OSM_BBox *box = osm_bbox_from_nodes(O->nodes);
    if (bbsize == 0.0)
        bbsize = auto_bbox(bbox_area(box), O->ways->num, O->nodes->num);

    if (debug)
        fprintf(stderr, "BBOX Size=%f\n", bbsize);

    osm_node_list_sort(O->nodes);
    if (debug)
        fprintf(stderr, "nodes sorted after %d\n", (int)(time(NULL)-start));
    // for (i=0; i<O->nodes->num; i++)
    //     fprintf(stderr, "NODE=% 16lu pos=% 10d\n", O->nodes->data[i]->id, i);

    double left, right, top, bottom;
    for (left=box->left_lon; left <= box->right_lon; left += bbsize) {
        for (bottom=box->bottom_lat; bottom <= box->top_lat; bottom += bbsize) {
            right = left + bbsize;
            top   = bottom + bbsize;

            int k, l;
            for (k=0; k<O->ways->num; k++) {
                uint64_t *nodes = O->ways->data[k]->nodes;
                l=0; 
                while (nodes[l]) {
                    long int pos = osm_node_pos(O->nodes, nodes[l]);
                    if (pos == -1) {
                        fprintf(stderr, "node %lu missing, referenced by way %lu\n", 
                                        nodes[l],  O->ways->data[k]->id);
                        ++l;
                        continue;
                    }
                    if (O->nodes->data[pos]->lon >= left   &&
                        O->nodes->data[pos]->lon <= right  &&
                        O->nodes->data[pos]->lat >= bottom &&
                        O->nodes->data[pos]->lat <= top)
                    {
                        osm_realloc_way_list(ways);
                        ways->data[ways->num] = O->ways->data[k];
                        ways->num += 1;
                        break;
                    }
                    ++l;
                }
            }
            if (debug)
                fprintf(stderr, "num ways in box %.7f,%.7f/%.7f,%.7f: %u\n", 
                        left,bottom, right,top, ways->num);
            for (i=0; i<ways->num; i++) {
                for (k=i+1; k<ways->num; k++) {
                    OSM_Way *way_i = ways->data[i];
                    OSM_Way *way_k = ways->data[k];
                    l = 1;
                    while (way_i->nodes[l]) ++l;
                    int num_i = l;
                    l = 0;
                    while (way_k->nodes[l]) ++l;
                    int num_k = l;
                    int r = 1;
                    int found = 0;
                    for (l=1; l< num_i -1; l++) {
                        for (r=1; r< num_k -1; r++) {
                            if (way_i->nodes[l] == way_k->nodes[r]) { /* same node */
                                if (way_i->nodes[l-1] == way_k->nodes[r-1] ||
                                    way_i->nodes[l-1] == way_k->nodes[r+1] ||
                                    way_i->nodes[l+1] == way_k->nodes[r-1] ||
                                    way_i->nodes[l+1] == way_k->nodes[r+1])
                                { /* prev or next node are the same */
                                    osm_realloc_way_list(dupes);
                                    dupes->data[dupes->num]   = way_i;
                                    dupes->data[dupes->num+1] = way_k;
                                    dupes->num += 2;
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        if (found) {
                            found = 0;
                            break;
                        }
                    }
                }
            }
            ways->num = 0;
            // osm_free_way_list(ways);
        }
    } // bbox check end
    
    fprintf(stderr, "finished searching dups after %d\n", (int)(time(NULL)-start));
    fprintf(stderr, "found %u duplicate ways\n", dupes->num);
    OSM_Data *D = malloc(sizeof(OSM_Data));
    D->nodes    = O->nodes;
    D->ways     = dupes;
    D->relations = NULL;

    if (gpx_file != NULL)
        write_gpx(D, gpx_file);

    return 0;
}

/* END */
