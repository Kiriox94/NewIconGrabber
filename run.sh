#!/bin/bash

NC='\033[0m' # No Color

echo "\033[43;30mStart compilation${NC}"
xmake
exitcode=$?

if [ $exitcode -eq 0 ]; then
    echo "\033[42;30mSuccessful compilation${NC}"
    echo "\033[45;30mSending nro to the switch...${NC}"

    nxlink build/cross/aarch64/debug/NewIconGrabber.nro -s -r 20
    echo
else
    echo "\033[41;97mCompilation failed${NC}" 
fi