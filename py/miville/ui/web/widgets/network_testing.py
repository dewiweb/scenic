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
# along with Miville. If not, see <http://www.gnu.org/licenses/>.

# System import
import time
import pprint

#App imports
from miville.ui.web.web import Widget, expose
from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *

log = log.start('debug', 1, 0, 'web_nettest')

class NetworkTesting(Widget):
    """
    Web widget for network performance testing with remote contacts.
    """
    def rc_start_test(self, contact_name, bandwidth=10, duration=1, kind="dualtest", unit='M'):
        # default values 
        caller = self
        #bandwidth = 10 # M
        #duration = 1 # s
        #kind = "dualtest"
        #log.debug("trying to get contact object for %s" % (contact_name))
        #log.debug(type(contact_name))
        try:
            contact_obj = self.api.get_contact(contact_name) # we need the object, not the name
        except AddressBookError, e:
            log.error("AddressBookError %s" % e.message)
        else:
            log.debug("widget is trying to start network testing with %s (b=%s, t=%s, k=%s, u=%s)" % (contact_obj, bandwidth, duration, kind, unit))
            self.api.network_test_start(caller, bandwidth, duration, kind, contact_obj, unit)
        return False # we must do this for rc_* methods
        
    def rc_stop_test(self, contact):
        log.debug("network testing stop is not implemented yet")
        return False # we must do this for rc_* methods
        
    def cb_network_test_start(self, origin, data):
        # data is a string
        # data could be {message, contact_name, duration}
        log.debug("started network test" + str(origin) + str(data))
        #log.debug("origin:" + str(origin))
        #log.debug("data:" + str(data))

    def cb_network_test_done(self, origin, data):
        """
        Results of a network test. 
        See network.py
        :param data: a dict with iperf statistics
        """
        contact_name = data['contact'].name
        local_data = None
        remote_data = None
        if data.has_key('local'):
            local_data = data['local']
        if data.has_key('remote'):
            remote_data = data['remote']
        self.callRemote('test_results', contact_name, local_data, remote_data)
    
    def cb_network_test_error(self, origin, data):
        """
        :param data: dict or string

        Example ::
        self.api.notify(
            caller, 
            {
            'msg':'Connection failed',
            'exception':'%s' % err,
            }, 
            "error")
        """
        txt = ""
        if isinstance(data, dict):
            msg = "Network Test Error: "
            # mandatory arguments
            for k in data.keys():
                msg += "  %s\n" % (data[k])
            log.error(msg)
            txt = msg
        else:
            log.error(data)
            txt = data
        self.callRemote('nettest_error', txt) # data)
        log.debug("self.callRemote('nettest_error', txt)")
                
    expose(locals())
