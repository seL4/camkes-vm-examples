#!/bin/sh
#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

# Print an error message to capture error condition
trap "if [ \$? -ne 0 ]; then echo 'Exited with error'; fi" 0;


server tcp://eth0:5555 1 10000


sub tcp://192.168.1.1:5555 5


worker tcp://192.168.1.1:5556 tcp://192.168.1.3:5556 &
work1=$!
worker tcp://192.168.1.1:5556 tcp://192.168.1.3:5556 &
work2=$!

(sleep 15 && kill $work1 && kill $work2) &
