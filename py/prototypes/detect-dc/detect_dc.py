#!/usr/bin/env python

import pprint
import re

DEVICE_OUTPUT = open("dc-output.txt", "r").read()

def parse_dc_cameras(text):
    dc_devices = {}
    sizes = []
    pmodes = []
    frame_rates = []
    for line in text.splitlines():
        if line.startswith("DC1394 Camera"):
            name = " ".join(line.split()[3:])
            current_dc_device = name
            dc_devices[name] = {
                "name" : name,
                "sizes" : [],
                "pmodes" : [],
                "frame_rates" : [],
            }
        else:
            mode = parse_dc_vmodes(line)
            if mode:
                sizes.append(mode[0])
                pmodes.append(mode[1])
            rates = parse_dc_framerates(line)
            if rates:
                frame_rates.append(rates)
            dc_devices[current_dc_device]["sizes"] = sizes
            dc_devices[current_dc_device]["pmodes"] = pmodes
            dc_devices[current_dc_device]["frame_rates"] = frame_rates
    return dc_devices

def parse_dc_vmodes(line):
    """
    Parse video modes
    i.e. 640x480_MONO8 (vmode 69)
    """
    # TODO: do we need to know the vmode number actually?
    # anyways, let's extract the resolution and pixel format
    # regex: 4 spaces (3 digits)x(3 digits)_(2-4 letters)(1-3 digits)
    vformat = re.compile(r"^\s{4}([0-9]{3}x[0-9]{3})_([A-Z]{2,4}[0-9]{1,3})")
    result = vformat.match(line)
    # we get tuples of resolution, pixel format
    if result:
        return result.groups()
    else:
        return None

def parse_dc_framerates(line):
    """
    parse framerates
    i.e. Framerates: 3.75,7.5,15.30
    """
    _framerates = re.compile("Framerates")
    result = _framerates.search(line)
    if result:
        frates_as_string = line.split()[1]
        frates = frates_as_string.split(",")
        return frates
    else:
        return None

if __name__ == "__main__":
    pprint.pprint(parse_dc_cameras(DEVICE_OUTPUT))
