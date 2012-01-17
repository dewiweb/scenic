all: sharedvideoreader sharedvideowriter clutterdemo

sharedvideoreader: shared-video-reader.c
	gcc -Wall shared-video-reader.c -o shared-video-reader `pkg-config --cflags --libs gstreamer-0.10`

sharedvideowriter: shared-video-writer.c
	gcc -Wall shared-video-writer.c -o shared-video-writer `pkg-config --cflags --libs gstreamer-0.10`

clutterdemo: clutterdemo.cpp
	g++ -Wall -g clutterdemo.cpp -o clutterdemo  `pkg-config --cflags --libs clutter-gst-1.0`

clean:
	rm -f shared-video-reader shared-video-writer clutterdemo *.o

