/*
 * open.c - open file... 
 *        
 * This file is licenced licenced under the General Public License 3. 
 *
 * Hanno Hecker <vetinari+osm at ankh-morp dot org>
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "osm.h"

static enum OSM_File_Type check_suffix(const char *filename) {
    size_t len;
    char *suffix;
    
    len = strlen(filename);
    if (len > 8) {
        suffix = (char *)filename + len - 8;
        if (strcmp(".osm.pbf", suffix) == 0)
            return OSM_FTYPE_PBF; 
    }
    if (len > 4) {
        suffix = (char *)filename + len - 4;
        if (strcmp(".osm", suffix) == 0)
            return OSM_FTYPE_XML;
    }
   
    return OSM_FTYPE_UNKNOWN;
}

static enum OSM_File_Type check_content(FILE *file) {
    char buffer[64];
    size_t len;
    enum OSM_File_Type type;

    type = OSM_FTYPE_PBF;
    len = fread(buffer, 1, 63, file);
    if (len == 63) {
        if (strncmp("<?xml version='1.0' encoding='UTF-8'?>", buffer, 38) == 0)
            type = OSM_FTYPE_XML;
    }
    else 
        type = OSM_FTYPE_UNKNOWN;

    fseek(file, 0, SEEK_SET);
    return type;
}

OSM_File *osm_open(const char *filename, enum OSM_File_Type type) {
    FILE *file;

    if (!*filename) {
        fprintf(stderr, "no file name given\n");
        return (OSM_File *)NULL;
    }
    if (type == OSM_FTYPE_UNKNOWN)
        type = check_suffix(filename);

    file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "failed to open '%s': %s\n", filename, strerror(errno));
        return (OSM_File *)NULL;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "file not seekable...: %s\n", strerror(errno));
        fclose(file);
        return (OSM_File *)NULL;
    }

    if (type == OSM_FTYPE_UNKNOWN)
        type = check_content(file);

    if (type == OSM_FTYPE_UNKNOWN) {
        fprintf(stderr, "unknown file type\n"); 
        fclose(file);
        return (OSM_File *)NULL;
    }
    
    OSM_File *osm_file = malloc(sizeof(OSM_File));
    if (osm_file == NULL) {
        fprintf(stderr, "failed to malloc: %s\n", strerror(errno));
        fclose(file);
        return (OSM_File *)NULL;
    }
    
    osm_file->type = type;
    osm_file->file = file;
    return osm_file;
}

/* END */
