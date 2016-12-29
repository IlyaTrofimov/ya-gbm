#!/usr/bin/env python 
import os
import sys

for line in sys.stdin:
	sys.stdout.write(line.strip())
	sys.stdout.write("\n")
