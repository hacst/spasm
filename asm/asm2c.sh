#!/usr/bin/env sh
if [ $# -ne 1 ]
then
	echo "Usage: $0 <asmfile>"
	exit 1
fi

# ./ldnasmp $1 0x08048080 0x10000000 0x15000000 0x20000000
./ldnasm $1

objdump -Mintel -D `basename $1 .asm` | ./sections2shell.py --header
objdump -Mintel -D `basename $1 .asm` | ./sections2shell.py

