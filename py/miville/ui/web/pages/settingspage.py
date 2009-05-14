#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
This module is an html page for settings information


"""


##System imports
#import os.path as path
#import os
#from xml.etree import ElementTree
#
##Twisted imports
#from twisted.internet import reactor
#from twisted.python.modules import getModule

try:
#    from nevow import loaders, appserver, static, tags, inevow
#    from nevow.athena import LivePage, LiveFragment, expose as nevow_expose
#    from nevow.i18n import render as i18nrender
#    from nevow.i18n import _, I18NConfig
    from nevow import rend
except ImportError:
    raise ImportError, 'If you want to use the Web interface, you need to install Nevow.'

#App imports
from miville.utils import Observer, log
from miville.utils.i18n import to_utf
from miville.utils.common import find_callbacks

from miville.settings import global_settings
from miville.settings import media_settings
from miville.settings import Settings
from miville.errors import *


log = log.start('debug', 1, 0, 'web')

def print_all_media_settings(media_settings):
    txt = "<h1>All MEDIA SETTINGS...</h1>"
    txt += "<ul>"
    for k, v in media_settings.iteritems():
        txt += "<li> [" + str(k) + "] " + v.name + "</li>"
        txt += "<ul>"
        for key, value in v.settings.iteritems():
            txt += "<li>  %s : %s</li>" % (key, str(value))
        txt += "</ul>"
    txt += "</ul>"
    return txt    

def print_media_setting(media_setting):
    txt = "<h5>[" + str(media_setting.id) + "] media setting \"" + media_setting.name + "\"</h5>"
    for key, value in media_setting.settings.iteritems():
        txt += "<p>       %s : %s</p>" % (key, str(value))
    return txt

def print_stream(stream):
    txt = " <h4>[" + stream.name + "] stream</h4>"
    txt += "<p>    enabled: " + str(stream.enabled) + "</p>"
    txt += "<p>    sync: " + stream.sync_group + "</p>"
    txt += "<p>    port: " + str(stream.port) + "</p>"
    #txt += "<p>    media setting: %s\n" % str(stream.setting)
    return txt

def print_stream_sub_group(group_id, group):
    txt = " <h3>[" + str(group_id) + "] stream sub group " + group.name + "</h3>"
    txt += "<p>   enabled: " + str(group.enabled) + "</p>"
    txt += "<p>   mode: " + str(group.mode) + "</p>"
    return txt

def print_global_setting(global_setting_id, global_setting):
    txt = "<h2>[" + str(global_setting_id) + "] global setting  " + global_setting.name + "</h2>"
    return txt
    
def print_settings():

    txt = "<h1>GLOBAL SETTINGS:</h1>"
    for k, global_setting in global_settings.iteritems():
        txt += print_global_setting(k, global_setting)
        for group_id,group in global_setting.stream_subgroups.iteritems():
            txt += print_stream_sub_group(group_id, group)
            for stream in group.media_streams:
                txt += print_stream(stream)                
                try:
                    id = stream.setting
                    media_setting = Settings.get_media_setting_from_id(id)
                    txt += print_media_setting(media_setting)
                except:
                    txt += "<p>     media setting %s (not available)</p>" % stream.setting
                    
    txt += print_all_media_settings(media_settings)
    return txt


class SettingsPage(rend.Page):
    def renderHTTP(self, ctx):
        
        beau_body =  print_settings()
        
        html = u"""
        <html>
        <head></head>
        <body>
        
            """
        html += beau_body     
        
        html += u"""
        
        </body>
        </html>
        """ 
        return str(html)


    
    
