#!/bin/sh
# to use within scratchbox, export SBOX_USE_CCACHE=no first.
make clean
./autogen.sh --enable-coverage
make
make check
lcov --directory tests/ --base-directory tests/ --capture --output-file lcov.info
genhtml lcov.info -o coverage/
