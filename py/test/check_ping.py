#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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
Starts two miville and tests the protocols/pinger module.
"""
import shutil
import unittest
from test import lib_miville_telnet as libmi

local = libmi.MivilleTester(use_tmp_home=True)
local.start_miville_process()
local.start_telnet_process()

remote = libmi.MivilleTester(use_tmp_home=True, port_offset=1)
remote.start_miville_process()
remote.start_telnet_process()

class Test_Ping(unittest.TestCase):
    def setUp(self):
        global local
        global remote
        remote.unittest = self
        local.unittest = self
        self.local = local
        self.remote = remote

    def test_01_ping(self):
        self.local.telnet_process.sendline("c -a Charlotte 127.0.0.1 2223")
        self.local.sleep(0.1)
        self.remote.telnet_process.sendline("c -a Pierre 127.0.0.1 2222")
        self.remote.sleep(0.1)
        self.local.telnet_process.sendline("c -s Charlotte")
        self.local.sleep(0.1)
        self.local.telnet_process.sendline("j -s")
        # TODO: test do you accept
        self.remote.telnet_process.sendline("Y")
        self.remote.sleep(0.1)
        self.local.expectTest('accepted', 'Connection not successful.')
        self.local.telnet_process.sendline("ping")
        self.local.expectTest('pong', 'Did not receive pong answer.')

