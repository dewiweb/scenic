LIB_SCENIC_SHARED_VIDEO_SRC_FILES = shared-video-writer.cpp shared-video-reader.cpp
LIB_SCENIC_SHARED_VIDEO_O_FILES = $(LIB_SCENIC_SHARED_VIDEO_SRC_FILES:%.cpp=%.o)

DEPS=`pkg-config --cflags --libs gstreamer-0.10 gio-2.0`

all: libscenicsharedvideo.so writer_example reader_example clutter_reader osg_reader data_writer data_reader data_reader_to_rtp data_reader_from_rtp

libscenicsharedvideo.so: $(LIB_SCENIC_SHARED_VIDEO_O_FILES)
	rm -f $@
	ld -G $(LIB_SCENIC_SHARED_VIDEO_O_FILES) -o $@

%.o: %.cpp shared-video.h
	gcc -fPIC -c -Wall $< -o $@ $(DEPS)

writer_example: libscenicsharedvideo.so writer-example.cpp
	g++ -Wall writer-example.cpp -o writer_example -L. -lscenicsharedvideo $(DEPS)

reader_example: libscenicsharedvideo.so reader-example.cpp
	g++ -Wall reader-example.cpp -o reader_example -L. -lscenicsharedvideo $(DEPS)

clutter_reader: reader-clutter-actor.cpp shared-video.h
	g++ -Wall reader-clutter-actor.cpp -o reader_clutter_actor -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 clutter-1.0 glew`

osg_reader: reader-osg.cpp shared-video.h
	g++ -Wall reader-osg.cpp -o reader_osg -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 openscenegraph gio-2.0 gstreamer-app-0.10`

data_writer: data-writer-example.cpp shared-video.h
	g++ -Wall data-writer-example.cpp -o data_writer_example -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

data_reader: data-reader-example.cpp shared-video.h
	g++ -Wall data-reader-example.cpp -o data_reader_example -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

data_reader_to_rtp: data-reader-to-rtp-example.cpp
	g++ -Wall data-reader-to-rtp-example.cpp -o data_reader_to_rtp -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

data_reader_from_rtp: data-reader-from-rtp-example.cpp
	g++ -Wall data-reader-from-rtp-example.cpp -o data_reader_from_rtp -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

clean:
	rm -f *.o *.so writer_example reader_example reader_osg reader_clutter_actor data_writer_example data_reader_example *~ data_reader_to_rtp data_reader_from_rtp
