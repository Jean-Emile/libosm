/*
 * osm-data.h - libosm data structures 
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#ifndef _OSM_DATA_H
# define _OSM_DATA_H 

// #include <inttypes.h>
#include <stdint.h>
typedef struct _osm_tag    OSM_Tag;
typedef struct _osm_tag_list OSM_Tag_List;
typedef struct _osm_node   OSM_Node;
typedef struct _osm_node_list OSM_Node_List;
typedef struct _osm_way    OSM_Way;
typedef struct _osm_way_list OSM_Way_List;
typedef struct _osm_rel_member OSM_Rel_Member;
typedef struct _osm_rel_member_list OSM_Rel_Member_List;
typedef struct _osm_relation OSM_Relation;
typedef struct _osm_rel_list OSM_Relation_List;
typedef struct _osm_data OSM_Data;
typedef struct _osm_bbox OSM_BBox;

struct _osm_bbox {
    double left_lon;
    double bottom_lat;
    double right_lon;
    double top_lat;
};

struct _osm_tag {
    char *key;
    char *val;
};

struct _osm_tag_list {
    uint32_t  size;
    uint32_t  num;
    OSM_Tag  *data;
};

struct _osm_node {
    uint64_t     id;
    double       lon;
    double       lat;
    char        *user;
    uint32_t     uid;
    uint32_t     version;    
    uint64_t     changeset;
    uint64_t     timestamp;
    OSM_Tag_List *tags;
};

struct _osm_way {
    uint64_t     id;
    char        *user;
    uint32_t     uid;
    uint32_t     version;
    uint64_t     changeset;
    uint64_t     timestamp;
    uint64_t     *nodes;
    OSM_Tag_List *tags;
};

struct _osm_way_list {
    uint32_t  size;
    uint32_t  num;
    OSM_Way  **data;
};

struct _osm_node_list {
    uint32_t  size;
    uint32_t  num;
    OSM_Node  **data;
};



enum {
    OSM_REL_MEMBER_TYPE_UNKNOWN  = 0,
    OSM_REL_MEMBER_TYPE_NODE     = 1,
    OSM_REL_MEMBER_TYPE_WAY      = 2,
    OSM_REL_MEMBER_TYPE_RELATION = 3
} OSM_Rel_Member_Type;

struct _osm_rel_member {
    uint16_t type;
    uint64_t ref;
    char *role;
};

struct _osm_rel_member_list {
    uint32_t  size;
    uint32_t  num;
    OSM_Rel_Member  *data;
};


struct _osm_relation {
    uint64_t        id;
    char      *user;
    uint32_t        uid;
    uint32_t        version;
    uint64_t        changeset;
    uint64_t        timestamp;
    OSM_Rel_Member_List  *member;
    OSM_Tag_List   *tags;
};

struct _osm_rel_list {
    uint32_t      size;
    uint32_t      num;
    OSM_Relation **data;
};

struct _osm_data {
    OSM_Node_List     *nodes;
    OSM_Way_List      *ways;
    OSM_Relation_List *relations;
//    OSM_CSet_List     *changesets;
};

#endif /* _OSM_DATA_H */
