#!/bin/bash
export PATH=$PATH:/usr/share/openvswitch/scripts
export PS1="$(cat /proc/cmdline | awk '{print $1}') $ "
