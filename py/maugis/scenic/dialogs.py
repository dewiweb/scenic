#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.
"""
GTK Dialogs well integrated with Twisted.
 * Error dialog
 * Yes/No dialog
"""
if __name__ == "__main__":
    from twisted.internet import gtk2reactor
    gtk2reactor.install() # has to be done before importing reactor
from twisted.internet import reactor
from twisted.internet import defer
import gtk

class ErrorDialog(object):
    """
    Error dialog. Fires the deferred given to it once done.
    Use the create static method as a factory.
    """
    def __init__(self, deferred, message, parent=None):
        """
        @param deferred: L{Deferred}
        @param message: str
        """
        self.deferredResult = deferred
        parent = None
        error_dialog = gtk.MessageDialog(
            parent=parent, 
            flags=0, 
            type=gtk.MESSAGE_ERROR, 
            buttons=gtk.BUTTONS_CLOSE, 
            message_format=message)
        error_dialog.set_modal(True)
        error_dialog.connect("close", self.on_close)
        error_dialog.connect("response", self.on_response)
        error_dialog.show()

    @staticmethod
    def create(message, parent=None):
        """
        Returns a Deferred which will be called with a True result.
        @param message: str
        @rettype: L{Deferred}
        """
        d = defer.Deferred()
        dialog = ErrorDialog(d, message, parent)
        return d

    def on_close(self, dialog, *params):
        print("on_close %s %s" % (dialog, params))

    def on_response(self, dialog, response_id, *params):
        #print("on_response %s %s %s" % (dialog, response_id, params))
        if response_id == gtk.RESPONSE_DELETE_EVENT:
            print("Deleted")
        elif response_id == gtk.RESPONSE_CANCEL:
            print("Cancelled")
        elif response_id == gtk.RESPONSE_OK:
            print("Accepted")
        self.terminate(dialog)

    def terminate(self, dialog):
        dialog.destroy()
        self.deferredResult.callback(True)

class YesNoDialog(object):
    """
    Yes/no confirmation dialog.
    Use the create static method as a factory.
    """
    def __init__(self, deferred, message, parent=None):
        self.deferredResult = deferred
        yes_no_dialog = gtk.MessageDialog(
            parent=parent, 
            flags=0, 
            type=gtk.MESSAGE_QUESTION, 
            buttons=gtk.BUTTONS_YES_NO, 
            message_format=message)
        yes_no_dialog.set_modal(True)
        yes_no_dialog.connect("close", self.on_close)
        yes_no_dialog.connect("response", self.on_response)
        yes_no_dialog.show()

    @staticmethod
    def create(message, parent=None):
        """
        Returns a Deferred which will be called with a boolean result.
        @param message: str
        @rettype: L{Deferred}
        """
        d = defer.Deferred()
        dialog = YesNoDialog(d, message, parent)
        return d

    def on_close(self, dialog, *params):
        print("on_close %s %s" % (dialog, params))

    def on_response(self, dialog, response_id, *params):
        print("on_response %s %s %s" % (dialog, response_id, params))
        if response_id == gtk.RESPONSE_DELETE_EVENT:
            print("Deleted")
            self.terminate(dialog, False)
        elif response_id == gtk.RESPONSE_NO:
            print("Cancelled")
            self.terminate(dialog, False)
        elif response_id == gtk.RESPONSE_YES:
            print("Accepted")
            self.terminate(dialog, True)

    def terminate(self, dialog, answer):
        dialog.destroy()
        self.deferredResult.callback(answer)


if __name__ == '__main__': 
    d = ErrorDialog.create('BOBBBBBBBBBB')
    d.addCallback(lambda result: reactor.stop())
    reactor.run()
