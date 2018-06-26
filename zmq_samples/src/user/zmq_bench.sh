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

echo "Running client server scenario"
server tcp://lo:5555 1 10000 &
client tcp://127.0.0.1:5555 1 10000
wait

echo "Running pub/sub scenario"
sub tcp://127.0.0.1:5555 5 &
pub tcp://lo:5555 1 6
wait

echo "Running pipeline scenario"
sink tcp://127.0.0.1:5556 100 &
worker tcp://127.0.0.1:5555 tcp://127.0.0.1:5556 &
worker tcp://127.0.0.1:5555 tcp://127.0.0.1:5556 &
# source is a built in command, so we need to use its actual path
/bin/source tcp://127.0.0.1:5555 tcp://127.0.0.1:5556 100
wait %1
kill %2
kill %3

echo "Finished running all scenarios"