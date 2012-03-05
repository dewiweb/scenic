LIB_SCENIC_SHARED_VIDEO_SRC_FILES = shared-video-writer.cpp shared-video-reader.cpp
LIB_SCENIC_SHARED_VIDEO_O_FILES = $(LIB_SCENIC_SHARED_VIDEO_SRC_FILES:%.cpp=%.o)

DEPS=`pkg-config --cflags --libs gstreamer-0.10 gio-2.0`

all: libscenicsharedvideo.so video_writer video_reader clutter_reader osg_reader data_writer data_reader data_reader_to_rtp data_reader_from_rtp data_writer_from_rtp pcd_writer pcd_reader

libscenicsharedvideo.so: $(LIB_SCENIC_SHARED_VIDEO_O_FILES)
	rm -f $@
	ld -G $(LIB_SCENIC_SHARED_VIDEO_O_FILES) -o $@

%.o: %.cpp shared-video.h
	gcc -fPIC -c -Wall $< -o $@ $(DEPS)

video_writer: libscenicsharedvideo.so video-writer.cpp
	g++ -Wall video-writer.cpp -o video_writer -L. -lscenicsharedvideo $(DEPS)

video_reader: libscenicsharedvideo.so video-reader.cpp
	g++ -Wall video-reader.cpp -o video_reader -L. -lscenicsharedvideo $(DEPS)

clutter_reader: clutter-actor-reader.cpp shared-video.h
	g++ -Wall clutter-actor-reader.cpp -o clutter_actor_reader -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 clutter-1.0 glew`

osg_reader: osg-reader.cpp shared-video.h
	g++ -Wall osg-reader.cpp -o osg_reader -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 openscenegraph gio-2.0 gstreamer-app-0.10`

data_writer: data-writer.cpp shared-video.h
	g++ -Wall data-writer.cpp -o data_writer -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

data_reader: data-reader.cpp shared-video.h
	g++ -Wall data-reader.cpp -o data_reader -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

data_reader_to_rtp: data-reader-to-rtp.cpp
	g++ -Wall data-reader-to-rtp.cpp -o data_reader_to_rtp -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

data_writer_from_rtp: data-writer-from-rtp.cpp
	g++ -Wall data-writer-from-rtp.cpp -o data_writer_from_rtp -L. -lscenicsharedvideo $(DEPS)

data_reader_from_rtp: data-reader-from-rtp.cpp
	g++ -Wall data-reader-from-rtp.cpp -o data_reader_from_rtp -L. -lscenicsharedvideo `pkg-config --cflags --libs gstreamer-0.10 gio-2.0 gstreamer-app-0.10`

pcd_writer: pcd_writer.cpp
	g++ -o pcd_writer pcd_writer.cpp -L. -lscenicsharedvideo `pkg-config --cflags --libs pcl_visualization-1.4 gstreamer-0.10 gio-2.0 gstreamer-app-0.10` -I /usr/include/vtk-5.6/

pcd_reader: pcd_reader.cpp
	g++ -o pcd_reader pcd_reader.cpp -L. -lscenicsharedvideo `pkg-config --cflags --libs pcl_visualization-1.4 gstreamer-0.10 gio-2.0 gstreamer-app-0.10` -I /usr/include/vtk-5.6/

clean:
	rm -f *.o *.so video_writer video_reader osg_reader clutter_actor_reader data_writer data_reader *~ data_reader_to_rtp data_reader_from_rtp data_writer_from_rtp pcd_reader pcd_writer
