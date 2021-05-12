#!/bin/sh
#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

set -e

# Print an error message to capture error condition
trap "if [ \$? -ne 0 ]; then echo 'Exited with error'; fi" 0;

echo "Running 3 scenarios on one vm"
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

echo "Running 3 scenarios accross 3 vms"
echo "Running client server scenario"
client tcp://192.168.1.2:5555 1 10000

# Sleep to allow subscriber to start
sleep 1

echo "Running pub/sub scenario"
pub tcp://192.168.1.1:5555 1 6

echo "Running pipeline scenario"

# Sleep to allow workers to start
sleep 1
/bin/source tcp://192.168.1.1:5556 tcp://192.168.1.3:5556 100
