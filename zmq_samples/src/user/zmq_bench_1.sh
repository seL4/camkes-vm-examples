#!/bin/sh
#
# Copyright 2018, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(DATA61_BSD)
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
