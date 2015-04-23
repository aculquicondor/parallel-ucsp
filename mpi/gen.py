#!/usr/bin/env python2

from random import randint
from sys import argv

if len(argv) < 1 or not argv[1].isdigit():
    exit(1)

n = int(argv[1])
print n
s = ''
for x in range(n):
    s += str(randint(0, 100000)) + ' '
print s
