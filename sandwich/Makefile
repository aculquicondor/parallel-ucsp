CC=g++ -O2
MPICC=mpicxx -O2

all: sandwich_parallel sandwich_serial gen

sandwich_parallel: sandwich_parallel.cc
	$(MPICC) sandwich_parallel.cc -o sandwich_parallel

sandwich_serial: sandwich_serial.cc
	$(CC) sandwich_serial.cc -o sandwich_serial

gen: gen.cc
	$(CC) gen.cc -o gen

clear:
	rm -f sandwich_parallel sandwich_serial gen
