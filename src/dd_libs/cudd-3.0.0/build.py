#!/usr/bin/env python

import os

def main():
    os.system("./configure --enable-obj --enable-dddmp \"CFLAGS=-m64 -Wall -Wextra -g -O3\" \"CXXFLAGS=-m64 -Wall -Wextra -g -std=c++14 -O3\" \"LDFLAGS=-m64\"")

if __name__ == "__main__":
    main()
