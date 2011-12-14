#!/usr/bin/env python

#
# Usage readelf -x <section> <elfexecutable> | ./rehex2sh.py
#

import sys

if __name__ == "__main__":
    hexstring = ''
    for line in sys.stdin.readlines():
        components = line.strip().split(' ', 5)
        if len(components) == 6 and components[0].startswith("0x"):
            hexstring += ''.join(components[1:5])
            
    # Reformat
    bytesperline = 10 
    shellcode = ''
    
    for i in xrange(0, len(hexstring), 2):
        if i != 0:
            shellcode += ', '
            if i % (bytesperline * 2) == 0:
                shellcode += '\n'

        hexbyte = hexstring[i:i+2]
        shellcode += '0x' + hexbyte

    print shellcode


