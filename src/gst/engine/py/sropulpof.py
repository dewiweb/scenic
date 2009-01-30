#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
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

from optparse import OptionParser

import sys

from milhouse import * # python bindings to our gst module

class PofExcept(Exception): pass

def parseArgs(args):
    """ Parse command line arguments """

    versionNum=str(PACKAGE_VERSION) + '\b' + str(RELEASE_CANDIDATE)

    parser = OptionParser(version="%prog " + versionNum)

    parser.add_option("-i", "--address", 
            dest="ip", default="127.0.0.1", help="provide ip address")
    parser.add_option("-v", "--videocodec", 
            dest="videoCodec", default="mpeg4", help="video codec: h264, h263 or mpeg4")
    parser.add_option("-a", "--audiocodec", 
            dest="audioCodec", default="raw", help="audio codec: vorbis, raw or mp3")
    parser.add_option("-k", "--videosink", 
            dest="videoSink", default="xvimagesink", help="videosink: xvimagesink or glimagesink")
    parser.add_option("-l", "--audiosink", 
            dest="audioSink", default="jackaudiosink", help="audiosink: jackaudiosink, alsasink or pulsesink")
    parser.add_option("-t", "--audioport", 
            type="int", dest="audioPort", default=AUDIO_PORT, help="audioport: 1024-60000")
    parser.add_option("-p", "--videoport", 
            type="int", dest="videoPort", default=VIDEO_PORT, help="videoport: 1024-60000")
    parser.add_option("-s", "--sender", 
            action="store_true", dest="isSender", help="designate this process as sender")
    parser.add_option("-r", "--receiver", 
            action="store_true", dest="isReceiver", help="designate this process as receiver")
    parser.add_option("-f", "--fullscreen", 
            action="store_true", dest="fullscreen", help="set videowindow to fullsceen (receiver side only)")
    parser.add_option("-d", "--videodevice", 
            dest="videoDevice", help="video4linux device to use: /dev/video0, /dev/video1 (sender side only)")
    parser.add_option("-n", "--screen", 
            type="int", dest="screenNum", default=0, help="xinerama screen number")
    parser.add_option("-c", "--numChannels", 
            type="int", dest="numChannels", default=2, help="number of channels (sender side only)")
    parser.add_option("-x", "--videosource",
            dest="videoSource", default="v4l2src", 
            help="gstreamer video source: v4l2src, dv1394src, videotestsrc, filesrc (sender side only)")
    parser.add_option("-y", "--audiosource",
            dest="audioSource", default="jackaudiosrc", 
            help="gstreamer audio source: jackaudiosrc, dv1394src, alsasrc, pulsesrc, audiotestsrc, filesrc (sender side only)")
    parser.add_option("-o", "--timeout",
            type="int", dest="timeout", default=0, 
            help="time in ms before stopping, 0 means play forever")

    return parser.parse_args(args)


def runAsReceiver(options):
    """ Receives media from a remote sender """
    vRx = buildVideoReceiver(options.ip, options.videoCodec, options.videoPort, options.screenNum, options.videoSink)
    aRx = buildAudioReceiver(options.ip, options.audioCodec, options.audioPort, options.audioSink)

    start()
    if options.fullscreen:
        vRx.makeFullscreen()

    eventLoop(options.timeout)
    wasPlaying = isPlaying()
    stop()
    return wasPlaying


def runAsSender(options):
    """ Sends media to a remote receiver """
    vConfig = None
    if options.videoDevice is None:
        vConfig = VideoSourceConfig(options.videoSource)
    else:
        vConfig = VideoSourceConfig(options.videoSource, options.videoDevice)
    
    vTx = buildVideoSender(vConfig, options.ip, options.videoCodec, options.videoPort)

    aConfig = AudioSourceConfig(options.audioSource, options.numChannels)
    aTx = buildAudioSender(aConfig, options.ip, options.audioCodec, options.audioPort)

    start() 
    
    assert(tcpSendBuffer(options.ip, VIDEO_CAPS_PORT, VIDEO_MSG_ID, vTx.getCaps()))
    assert(tcpSendBuffer(options.ip, AUDIO_CAPS_PORT, AUDIO_MSG_ID, aTx.getCaps()))

    eventLoop(options.timeout)
    wasPlaying = isPlaying()
    stop()
    return wasPlaying



def run(myArgs):
    setHandler() # to catch interrupts at cpp level first
    options = parseArgs(myArgs)[0]
    
    if options.isSender:
        print "running as sender"
        return runAsSender(options)
    elif options.isReceiver:
        print "running as receiver"
        return runAsReceiver(options)
    else:
        raise PofExcept("Must specify if this process is a sender or receiver")


if __name__ == '__main__':
    run(sys.argv[1:])

