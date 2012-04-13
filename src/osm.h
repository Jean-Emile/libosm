/*
 * osm.h - libosm main include file
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#ifndef _OSM_H
# define _OSM_H

#define LIBOSM_VERSION "0.3"

#include <stdio.h> /* FILE */
#include "fileformat.pb-c.h"
#include "osmformat.pb-c.h"

#include "osm-data.h"

#define LINE_SIZE 4096
#define LIST_THRESHOLD 0.9

#define OSMDATA_NODE 0x01
#define OSMDATA_WAY  0x02
#define OSMDATA_REL  0x04
#define OSMDATA_CSET 0x08
#define OSMDATA_DUMP 0x10
#define OSMDATA_BBOX 0x20

#define NANO_DEGREE .000000001
#define MAX_BLOCK_HEADER_SIZE 64*1024
#define MAX_BLOB_SIZE 32*1024*1024

#define DEBUG_MEM 1

extern int debug;

enum OSM_File_Type {
    OSM_FTYPE_UNKNOWN,
    OSM_FTYPE_PBF,
    OSM_FTYPE_XML
};

typedef struct _osm_file {
    FILE *file;
    enum OSM_File_Type type;
} OSM_File;

/* util.c */
extern char *osm_relmember_type(int id);
extern void osm_init();
extern char *osm_encode_xml(char *src);
struct osm_members {
    uint32_t num;
    uint32_t size;
    uint64_t *data;
};
extern int osm_cmp_member(const void *a, const void *b);
extern void osm_sort_member(struct osm_members *m);
extern void osm_add_members(struct osm_members *m, uint32_t num, uint64_t *list, int sort);
extern int osm_is_member(struct osm_members *m, uint64_t id);


/* free.c */
extern void osm_free_tags(OSM_Tag_List *t);
extern void osm_free_node(OSM_Node *n);
extern void osm_free_way(OSM_Way *w);
extern void osm_free_way_list(OSM_Way_List *w);
extern void osm_free_relation(OSM_Relation *r);

/* realloc.c */
extern void osm_realloc_tag_list(OSM_Tag_List *t);
extern void osm_realloc_node_list(OSM_Node_List *n);
extern void osm_realloc_way_list(OSM_Way_List *w);
extern void osm_realloc_rel_list(OSM_Relation_List *r);
extern void osm_realloc_nodes(uint64_t *n, int num, int *size);
extern void osm_realloc_rel_member(OSM_Rel_Member_List *r);

/* xml.c */
extern uint64_t osm_timestamp2epoch(char *ts);
char *osm_xml_decode(char *src);
extern char *osm_xml_fetch_param(char *src, char *str, char *dest);
extern OSM_Data *osm_xml_parse(OSM_File *F,
              int mode,
              OSM_BBox *bbox,
              int (*node_filter)(OSM_Node *),
              int (*way_filter)(OSM_Way *),
              int (*rel_filter)(OSM_Relation *)/*,
              int (*cset_filter)(OSM_Changeset *) */
        );

/* xml-relation.c */
extern OSM_Relation *osm_xml_get_relation(FILE *file, char *buffer, char *param);
extern OSM_Relation_List *osm_xml_parse_relations(long int start,
                            FILE *file,
                            int mode,
                            int(*filter)(OSM_Relation *r),
                            struct osm_members *wanted);
/* xml-way.c */
extern OSM_Way *osm_xml_get_way(FILE *file, char *buffer, char *param);
extern OSM_Way_List *osm_xml_parse_ways(long int start,
                        FILE *file,
                        int mode,
                        int(*filter)(OSM_Way *w),
                        struct osm_members *wanted);
/* xml-node.c */
extern OSM_Node *osm_xml_get_node(FILE *file, char *buffer, char *param);
extern OSM_Node_List *osm_xml_parse_nodes(long int start,
                                    FILE *file,
                                    int mode,
                                    int(*filter)(OSM_Node *n),
                                    struct osm_members *wanted);

/* xml-write.c */
extern void osm_xml_write_header(char *who, FILE *outfh);
extern void osm_xml_write_footer(FILE *outfh);
extern void osm_xml_write_tags(OSM_Tag_List *t, FILE *outfh);
extern void osm_xml_write_node(OSM_Node *n, FILE *outfh);
extern void osm_xml_write_way(OSM_Way *w, FILE *outfh);
extern void osm_xml_write_relation(OSM_Relation *r, FILE *outfh);

/* pbf.c */
extern OSM_Data *osm_pbf_parse(OSM_File *F,
              uint32_t mode,
              OSM_BBox *bbox,
              int (*node_filter)(OSM_Node *),
              int (*way_filter)(OSM_Way *),
              int (*rel_filter)(OSM_Relation *) /*,
              int (*cset_filter)(OSM_Changeset *) */
        );

/* pbf-util.c */
extern void osm_pbf_timestamp(const long int deltatimestamp, char *timestamp);
extern unsigned char *osm_pbf_uncompress_blob(Blob *bmsg);
extern uint32_t osm_pbf_bh_length(OSM_File *F);
extern void osm_pbf_free_bh(BlockHeader *bh);
extern BlockHeader *osm_pbf_get_bh(OSM_File *F, uint32_t len);
extern void osm_pbf_free_blob(Blob *B, unsigned char *uncompressed);
extern Blob *osm_pbf_get_blob(OSM_File *F, uint32_t len, unsigned char **uncompressed);
extern void osm_pbf_free_primitive(PrimitiveBlock *P);
extern PrimitiveBlock *osm_pbf_unpack_data(Blob *B, unsigned char *uncompressed);

/* nodes.c */
extern int osm_node_pos(OSM_Node_List *n, uint64_t id);
extern int osm_node_cmp(const void *a, const void *b);
extern void osm_node_list_sort(OSM_Node_List *n);

/* bbox.c */
extern OSM_BBox *osm_bbox_from_nodes(OSM_Node_List *n);
/* open.c */
extern OSM_File *osm_open(const char *filename, enum OSM_File_Type type);
/* parse.c */
extern OSM_Data *osm_parse(OSM_File *F,
              int mode,
              OSM_BBox *bbox,
              int (*node_filter)(OSM_Node *),
              int (*way_filter)(OSM_Way *),
              int (*rel_filter)(OSM_Relation *)/*,
              int (*cset_filter)(OSM_Changeset *) */
        );

/* gpx-write.c */
extern uint64_t *osm_gpx_write_init(OSM_Data *data, uint32_t *num);
extern void osm_gpx_write_header(char *who, FILE *outfh);
extern void osm_gpx_write_footer(FILE *outfh);
extern void osm_gpx_write_tags(OSM_Tag_List *t, FILE *outfh);
extern void osm_gpx_write_node(OSM_Node *n, FILE *outfh, int is_trkpt);
extern void osm_gpx_write(OSM_Data *data, FILE *outfh, char *creator);

/* shortcuts */
#define osm_close(f) { fclose(f->file); free(f); }

#define trim_left(l) { while (*l && (*l == ' ' || *l == '\t')) ++l; }

#define FETCH_TAG(t, n) { \
        osm_realloc_tag_list(t); \
        str = osm_xml_fetch_param(line, "k", param); \
        if (str == NULL) continue; \
        t->data[ n ].key = strdup(str); \
        str = osm_xml_fetch_param(line, "v", param); \
        if (str == NULL) t->data[ n ].val = ""; \
        else t->data[ n ].val = osm_xml_decode(str); \
        t->num += 1; \
    }

#endif /* _OSM_H */
