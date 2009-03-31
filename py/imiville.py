
from pprint import pprint, pformat
import time
import sys

from twisted.internet import reactor 

from miville import core #  as miville
from miville.utils.observer import Observer
from miville.ui.cli import CliView
from miville import devices

"""
Miville for ipython.

See https://svn.sat.qc.ca/trac/miville/wiki/IPython    
"""

#TODO: override notify in order to add reactor.iterate() there.

def go(duration=0.1): # num=999
    """
    Runs the reactor for n seconds
    """
    end = time.time() + duration
    while time.time() < end:
        reactor.iterate()
    
class Update(object):
    """
    Represents a notification from miville's core api.
    """
    def __init__(self, origin, key, value):
        self.value = value
        self.origin = origin
        self.key = key
    
class IPythonController(object):
    def __init__(self):
        self.verbose = True
    def write (self, msg, prompt=False, endl=True):
        if self.verbose:
            print "%s" % (msg.encode('utf-8'))
        
    def write_prompt(self):
        pass

class IPythonView(CliView):
    """  
    ipython results printer. 
    The View (Observer) in the MVC pattern. 
    """
    def __init__(self, subject, controller):
        #Observer.__init__(self, subject)
        #self.controller = controller
        CliView.__init__(self, subject, controller)
        self.verbose = True
        
    def update(self, origin, key, value):
        global updates, last
        #if origin is self.controller:
        last = Update(origin, key, value)
        updates.append(last) # always appends new notifications
        if self.verbose:
            sys.stdout.write(get_color('CYAN'))
            print "-------------------------------------  update:  ------------------------------"
            print "KEY:    %s" % (str(key))
            print "ORIGIN: %s" % (str(origin))
            print "VALUE:  %s" % (pformat(value))
            print "------------------------------------------------------------------------------"
            sys.stdout.write(get_color('BLACK'))
            CliView.update(self, origin, key, value)

def get_color(c=None):
    """
    Returns ANSI escaped color code.
    
    Colors can be either 'BLUE' or 'MAGENTA' or None
    """
    # TODO: make this generically includable from both test/systest_telnet.py and here.
    if c == 'BLUE':
        s = '31m'
    elif c == 'CYAN':
        s = '36m'
    elif c == 'MAGENTA':
        s = '35m'
    else:
        s = '0m' # default (black or white)
    return "\x1b[" + s

updates = []
last = None
# core = None
api = None
me = None
view = None

def main():
    global api, me, view
    core.main(core.MivilleConfiguration())
    go(0.5)
    # core = miville.core
    api = core.core.api
    me = IPythonController()
    view = IPythonView(core.core, me)
    #print "iMiville is ready for anything."

if __name__ == '__main__':
    main()

