OSM_BINARY_PATH=../OSM-binary

SRC_FILES=open.c free.c realloc.c util.c parse.c \
	pbf-util.c pbf.c \
	xml.c xml-relation.c xml-way.c xml-node.c xml-write.c \
	nodes.c bbox.c \
	gpx-write.c \
	fileformat.pb-c.c osmformat.pb-c.c

OBJECT_FILES=open.o free.o realloc.o util.o parse.o \
	pbf-util.o pbf.o \
	xml.o xml-relation.o xml-way.o xml-node.o xml-write.o \
	nodes.o bbox.o \
	gpx-write.o \
	fileformat.pb-c.o osmformat.pb-c.o

GENERATED_FILES=fileformat.pb-c.c osmformat.pb-c.c \
                fileformat.pb-c.h osmformat.pb-c.h

EXEC_FILES=osmpbf2osm osm-extract osm2gpx waydupes
LIB_FILES=libosm.so

#CC_FLAGS=-Wall -g -pg
CC_FLAGS=-Wall -g -O2
LD_FLAGS=-lm -lprotobuf-c -lz


#%.o: %.c $(SRC_FILES) proto_c_gen
.c.o: $(SRC_FILES) proto_c_gen
	#$(CC) $(CC_FLAGS) -o $@ -c $*.c
	$(CC) -Wl,--export-dynamic -shared -fPIC $(CC_FLAGS) -o $@ -c $<

all: libosm.so osmpbf2osm osm-extract osm2gpx waydupes

libosm.so: proto_c_gen $(OBJECT_FILES) $(SRC_FILES)
	$(CC) -Wl,--export-dynamic -shared -fPIC $(CC_FLAGS) $(LD_FLAGS) \
		-o libosm.so $(OBJECT_FILES)
	#$(CC) $(CC_FLAGS) $(LD_FLAGS) 

osmpbf2osm: libosm.so
	#$(CC) $(CC_FLAGS) -o osmpbf2osm.o -c osmpbf2osm.c
	#$(CC) $(CC_FLAGS) $(LD_FLAGS) -o osmpbf2osm osmpbf2osm.o $(OBJECT_FILES)
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -L. -losm -o osmpbf2osm osmpbf2osm.c
	
osm-extract: libosm.so
	#$(CC) $(CC_FLAGS) -o osm-extract.o -c osm-extract.c
	#$(CC) $(CC_FLAGS) $(LD_FLAGS) -o osm-extract osm-extract.o $(OBJECT_FILES)
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -L. -losm -o osm-extract osm-extract.c

osm2gpx: libosm.so
	#$(CC) $(CC_FLAGS) -o osm2gpx.o -c osm2gpx.c
	#$(CC) $(CC_FLAGS) $(LD_FLAGS) -o osm2gpx osm2gpx.o $(OBJECT_FILES)
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -L. -losm -o osm2gpx osm2gpx.c

waydupes: libosm.so
	#$(CC) $(CC_FLAGS) -o waydupes.o -c waydupes.c
	#$(CC) $(CC_FLAGS) $(LD_FLAGS) -o waydupes waydupes.o $(OBJECT_FILES)
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -L. -losm -o waydupes waydupes.c

clean:
	rm -f $(OBJECT_FILES) $(GENERATED_FILES) proto_c_gen $(EXEC_FILES) $(LIB_FILES) 

proto_c_gen:
	protoc-c --proto_path=$(OSM_BINARY_PATH)/src \
		--c_out=. $(OSM_BINARY_PATH)/src/*.proto
	touch ./proto_c_gen

fileformat.pb-c.o: proto_c_gen
	#$(CC) $(CC_FLAGS) -o $@ -c $*.c
	$(CC) -Wl,--export-dynamic -shared -fPIC $(CC_FLAGS) -o $@ -c $*.c

osmformat.pb-c.o: proto_c_gen
	#$(CC) $(CC_FLAGS) -o $@ -c $*.c
	$(CC) -Wl,--export-dynamic -shared -fPIC $(CC_FLAGS) -o $@ -c $*.c

# vim: syn=make ts=8 sw=8 noexpandtab
