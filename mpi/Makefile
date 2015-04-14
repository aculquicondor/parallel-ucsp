MPICC=mpicxx -O2

all: builtin_broadcast own_broadcast

builtin_broadcast: builtin_broadcast.cc
	$(MPICC) builtin_broadcast.cc -o builtin_broadcast

own_broadcast: own_broadcast.cc
	$(MPICC) own_broadcast.cc -o own_broadcast

clear:
	rm -f builtin_broadcast own_broadcast

