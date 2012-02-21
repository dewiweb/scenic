#!/bin/bash

# could be replaced with autoreconf -fivI m4 (verbose, force rebuild of ltmain, .in files, etc.)
autoreconf --install --verbose

if [ $? != 0 ]; then
    echo "autoreconf return value is $?"
    exit 1
fi

if [ ! "x$LOGNAME" = "xbbslave" ]; then
    ./configure $@
else
    ./configure $@
fi

