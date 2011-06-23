Using the shared-video-0.6 library
----------------------------------

Scenic's shared-video library allows one to shared video between a milhouse receiver and any C++ program.

Launch milhouse:
 $ lunch -g ./milhouse-shared-video.lunch

Compile the osg and clutter versions of the app and run it:
 $ make
Run the osg verson:
 $ ./run sharedvideoexample
Run the osg verson:
 $ ./clutterrun sharedvideoexample

Where to find more examples: 
 * In the SPIN Framework: src/spin/SharedVideoTexture.cpp and include/SharedVideoTexture.h
   See https://github.com/mikewoz/spinframework
   Note: SPIN's SharedVideoNode is deprecated in favour of SharedVideoTexture.
 * In the Scenic inhouse prototypes: inhouse/prototypes/appsink
   Note: this is an old Subversion repository. http://code.sat.qc.ca/scenic/inhouse/

