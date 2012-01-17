--- Basic Gstreamer Pipeline
----------------------------
(framerate limited by CPU)

gst-launch-0.10 -v -m videotestsrc ! video/x-raw-yuv, width=1920, height=1080, framerate=30/1 ! gdppay ! shmsink socket-path=/tmp/testgdp5 shm-size=94967295

gst-launch shmsrc socket-path=/tmp/testgdp5 ! gdpdepay ! xvimagesink

--- testing with milhouse 
-------------------------
from current directory

1) enabling transmission
Sender:
../main/milhouse -s --disable-audio
Receiver:
../main/milhouse -r --videosink sharedvideosink --shared-video-id "/tmp/mysharedvideosocket" --disable-audio

2) rendering
gst-launch shmsrc socket-path=/tmp/mysharedvideosocket ! gdpdepay ! xvimagesink
OR
./shared-video-reader /tmp/mysharedvideosocket
OR
./clutterdemo /tmp/mysharedvideosocket
