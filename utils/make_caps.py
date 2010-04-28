#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.
#

import sys
try:
    import pygst
    pygst.require('0.10')
    import gst
except ImportError:
    print("import failed, please install gst-python")
    sys.exit(1)

import gobject
import sys

class Profile(object):
    """ Holds codec name, encoder names, payloader name and caps string """
    def __init__(self, encoder, payloader):
        self.encoder = encoder
        self.payloader = payloader
        self.caps = ''
    def __str__(self):
        return self.caps


class VideoProfile(Profile):
    PIXEL_ASPECT_RATIO_TABLE = {}
    # PAL
    PIXEL_ASPECT_RATIO_TABLE["720x576"] = {"4:3" :"59/54"}
    PIXEL_ASPECT_RATIO_TABLE["704x576"] = {"4:3" : "59/54", "16:9" : "118/81"}
    PIXEL_ASPECT_RATIO_TABLE["352x288"] = {"16:9" : "118/81"}

    # NTSC
    PIXEL_ASPECT_RATIO_TABLE["720x480"] = {"4:3" : "10/11"}
    PIXEL_ASPECT_RATIO_TABLE["704x480"] = {"4:3" : "10/11", "16:9" : "40/33"}
    PIXEL_ASPECT_RATIO_TABLE["352x240"] = {"16:9" : "40/33"}

    # Misc. used by us
    PIXEL_ASPECT_RATIO_TABLE["768x480"] = {"4:3" : "6/7"}
    PIXEL_ASPECT_RATIO_TABLE["640x480"] = {"4:3" : "1/1"}
    
    def get_pixel_aspect_ratio(self, picture_aspect_ratio):
        key = str(self.width) + "x" + str(self.height) 
        if key in self.PIXEL_ASPECT_RATIO_TABLE:
            if picture_aspect_ratio in self.PIXEL_ASPECT_RATIO_TABLE[key]:
                return self.PIXEL_ASPECT_RATIO_TABLE[key][picture_aspect_ratio]
            else: # default to square pixels
                return "1/1"
        else: # default to square pixels
            return "1/1"

    """ Holds codec name, encoder names, payloader name and caps string """
    def __init__(self, encoder, payloader, width=640, height=480, picture_aspect_ratio="4:3"):
        Profile.__init__(self, encoder, payloader)
        self.width = width
        self.height = height
        self.pixel_aspect_ratio = self.get_pixel_aspect_ratio(picture_aspect_ratio)
        # FIXME: this only works for yuv and doesn't handle other framerates
        self.src = "videotestsrc ! video/x-raw-yuv, width=%d, height=%d, framerate=30000/1001, pixel-aspect-ratio=%s " \
                    % (self.width, self.height, self.pixel_aspect_ratio) 

class AudioProfile(Profile):
    """ Holds codec name, encoder names, payloader name, num channels and caps string """
    def __init__(self, encoder, payloader, channels=2, rate=48000):
        Profile.__init__(self, encoder, payloader)
        self.channels = channels
        self.rate = rate
        self.src = "audiotestsrc ! audio/x-raw-int, channels=%d, rate=%d ! audioconvert " % (self.channels, self.rate)


def generate_caps(profile_name, profile):
    """ Generate caps for a set of profiles """
    #print ('/*----------------------------------------------*/' )
    #print ('CAPS FOR CODEC ' + codecName + ':')
    launch_line = profile.src + " ! %s ! %s name=payloader ! fakesink sync=false" \
    % (profile.encoder, profile.payloader)
    pipeline = gst.parse_launch(launch_line)
    pipeline.set_state(gst.STATE_PLAYING)
    mainloop = gobject.MainLoop()

    payloader = pipeline.get_by_name("payloader")
    srcpad = payloader.get_static_pad("src")

    caps = srcpad.get_negotiated_caps()

    while caps is None:
        caps = srcpad.get_negotiated_caps()
        
    profile.caps = caps.to_string().split(', ssrc')[0].strip()

    #print codec
    pipeline.set_state(gst.STATE_NULL)

    return profiles 


def save_caps(profiles, filename):
    """ Write codec/caps dict to file <filename> """
    try:
        """ Write to file """
        file = None
        filestr = ''
        if filename is not None:
            file = open(filename, 'w')
            filestr += '// ' + filename + ' generated by ' + __file__.strip('./') + '\n\n'
        else:
            file = sys.stdout
            filename = ''

        filestr += '#include "' + filename.split('cpp')[0].strip() + 'h"\n\n'
        filestr += '#include <boost/assign.hpp>\n'
        filestr += '#include <map>\n\n'

        filestr += 'std::string caps::getCaps(const std::string &key)\n{\n'
        filestr += '    using namespace boost::assign;\n'
        filestr += '    static std::map<std::string, std::string> names = map_list_of \n'
        
        delimiter = ''
        for profileName, profile in profiles.iteritems():
            filestr += delimiter 
            filestr += '        ("' + profileName + '" , '
            filestr += '"' + profile.caps.replace('\\', '\\\\').replace('"', '\\"') + '")'
            delimiter = '\n\n'

        filestr += ';\n\n'

        filestr += '    return names[key];\n}\n\n'

        file.write(filestr)

        file.flush()

    except IOError, e:
        sys.stderr.write(e)
    finally:
            file.close()


profiles = {}

encoders = {
    'mpeg4' : 'ffenc_mpeg4',
    'h264' : 'x264enc',
    'h263' : 'ffenc_h263p',
    'mp3' : 'lame',
    'raw' : 'identity silent=true'
}

payloaders = {
    'mpeg4' : 'rtpmp4vpay',
    'h264' : 'rtph264pay',
    'h263' : 'rtph263ppay',
    'mp3' : 'rtpmpapay',
    'raw' : 'rtpL16pay'
}


if __name__ == '__main__': 
    if len(sys.argv) == 2:
        filename = sys.argv[1]
    else:
        filename = None

RESOLUTIONS = ( 
                (160, 120),
                (176, 120), 
                (320, 240), 
                (352, 240), 
                (640, 480), 
                (704, 240),
                (704, 480),
                (720, 480),
                (768, 480),
                (800, 600), 
                (1024, 768), 
                (1280, 960))

for picture_aspect_ratio in ("4:3", "16:9"):
    for resolution in RESOLUTIONS:
        for codec in ('mpeg4', 'h264', 'h263'):
            profile_name = codec + '_%d_%d_%s' % (resolution[0], resolution[1], picture_aspect_ratio)
            profiles[profile_name] = VideoProfile(encoders[codec], payloaders[codec], resolution[0], resolution[1], picture_aspect_ratio)

SAMPLERATES = [16000, 22050, 32000, 44100, 48000]

for rate in SAMPLERATES:
    for codec in ('raw', 'mp3'):
        for channels in xrange(1, 3):
            profile_name = codec + '_%d_%d' % (channels, rate)
            profiles[profile_name] = AudioProfile(encoders[codec], payloaders[codec], channels, rate)

for profile_name, profile in profiles.iteritems():
    profile = generate_caps(profile_name, profile)

# generate caps by hand for raw because its more stable than getting them from a pipeline
codec = 'raw'
SAMPLERATES.append(96000)
for rate in SAMPLERATES:
    for channels in xrange(3, 9):
        profile_name = codec + '_%d_%d' % (channels, rate)
        profiles[profile_name] = AudioProfile(encoders[codec], payloaders[codec], channels, rate)
        profiles[profile_name].caps = 'application/x-rtp, media=(string)audio, clock-rate=(int)%d, encoding-name=(string)L16, encoding-params=(string)%d, channels=(int)%d, payload=(int)96' % (rate, channels, channels)

    
save_caps(profiles, filename)

