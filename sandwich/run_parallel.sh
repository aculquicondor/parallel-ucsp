#!/bin/sh

if [ $# -eq 0 ]; then
  exit 0
fi

tstart=`date +%s%N`
mpirun -np $1 sandwich_parallel
tend=`date +%s%N`
echo "TIME: $(((tend-tstart)/1000000)) ms"

