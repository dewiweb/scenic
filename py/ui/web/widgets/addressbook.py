# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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

#App imports
from ui.web.web import Widget, expose
from utils import log
from utils.i18n import to_utf

log = log.start('debug', 1, 0, 'web_adb')


class Addressbook(Widget):
    """
    """
#    help = {'adb_list':_('This is the list of contacts'),
#            'adb_add':_('Add a new contact to the list')
#            }
        
    def cb_get_contacts(self, origin, data):
        adb = []
        contacts = data[0].items()
        contacts.sort()
        for name, contact in contacts:
            adb.append((contact.name, to_utf(contact.address), contact.port))
        log.info('receive update: %r' % self)
        self.callRemote('updateList', adb)
        
    def rc_get_list(self):
        self.api.get_contacts(self)
        return False
        
    expose(locals())
