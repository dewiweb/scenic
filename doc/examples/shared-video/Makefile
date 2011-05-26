DEPS=clutter-1.0 shared-video-0.6 gstreamer-0.10 gstreamer-app-0.10 openscenegraph
CXXFLAGS=`pkg-config --cflags $(DEPS)` -Wall -Werror -O2
# -Wfatal-errors -Wextra 
LIBS=`pkg-config --libs $(DEPS)` -lboost_thread -lrt 
OBJS=osgview.o clutterexample.o

ifeq ($(shell uname -s),Darwin)
  CXXFLAGS += -DDARWIN
endif

all: run clutterrun

%.o : %.cpp %.h
	$(CC) -c $(CXXFLAGS) $< -o $@

# run: $(OBJS)
run: osgview.o
	g++ -Wall -g $(LIBS) $^ -o $@

clutterrun: clutterexample.o
	g++ -Wall -g $(LIBS) $^ -o $@

.PHONY: clean

clean:
	rm -f run *.o

