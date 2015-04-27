#!/bin/sh

tstart=`date +%s%N`
./sandwich_serial
tend=`date +%s%N`
echo "TIME: $(((tend-tstart)/1000000)) ms"

