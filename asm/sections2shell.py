#!/usr/bin/env python

#
# Usage: objdump -D <binary> | ./section2shell.py [--header] [<prefix>]
#

import re
import sys

def parse(lines, prefix):
    sectionHeadRegex = re.compile("^Disassembly of section \\.([^\s:]+):\s*$")
    symbolNameRegex = re.compile("^[\da-f]+\s<([^\s>]+)>:\s*$")
    sectionContentRegex = re.compile("^\s*[\da-f]+:\s*(([\da-f]{2}\s)+)\s*(.*)$")
    
    sections = {}
    
    section = None
    for line in lines:
        match = sectionHeadRegex.match(line)
        if match:
            section = match.group(1)
            if section.startswith(prefix):
                sections[section] = []
            else:
                section = None
        elif section:
            match = sectionContentRegex.match(line)
            if not match:
                match = symbolNameRegex.match(line)
                if match:
                    sections[section].append(([], match.group(1)))
                continue
            
            bytestr = match.group(1)
            bytes = [int(t, 16) for t in bytestr.strip().split()]
            code = match.group(3)
            
            sections[section].append((bytes, code))
            
    return sections

def generateCShellCode(sections, header = False, commentindent = 40, ):
    result = ""
    for section, codes in sections.iteritems():
        result += "const unsigned char " + section + "[]"
        
        if header:
            result += ";\n"
            continue
        
        result += " = {\n"
        
        for (bytes, comment) in codes:
            if not bytes:
                result += " " * commentindent + "/* " + comment + ": */\n"
                continue
            
            genline =  "    " + ', '.join([hex(byte) for byte in bytes]) + ","
            
            if len(genline) < commentindent:
                padding = ' ' * (commentindent - len(genline) % commentindent)
            else:
                padding = ' '
                
            result += genline + padding + "/* " + comment + " */\n"
        
        result += "};\n\n"
        
    return result

if __name__ == "__main__":
    prefix = "spasm_"
    header = False
    
    if len(sys.argv) > 1:
        i = 1
        while i < len(sys.argv):
            if sys.argv[i] == '--header':
                header = True
            else:
                break
            i += 1
        
        if i == len(sys.argv) - 1:
            prefix = sys.argv[i]
        elif i < len(sys.argv):
            print "Usage: objdump -D <binary> | ./section2shell.py [--header] [<prefix>]"
            sys.exit(1)
    
    print generateCShellCode(parse(sys.stdin.readlines(), prefix), header)