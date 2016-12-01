<!--

  Copyright 2016, Data61
  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
  ABN 41 687 119 230.
  This software may be distributed and modified according to the terms of
  the BSD 2-Clause license. Note that NO WARRANTY is provided.
  See "LICENSE_BSD2.txt" for details.
  @TAG(D61_BSD)

-->

camkes-arm-vm
=============

This repo is for running virtualised Linux on seL4 for arm platforms.

Currently the exynos5 and tk1 machines are supported.

## tk1 configuration

The tk1 currently uses a ramfs constructed using [Buildroot 2016.08.1](https://buildroot.org/downloads/) with the following config:
`apps/vm/linux/buildroot_tk1_defconfig`

When build root starts:
```
buildroot login: root
#
```

### wireless configuration
To configure the Ralink 2780 USB wifi dongle:

```
ifconfig wlan0 up
iw wlan0 scan
wpa_passphrase SSID_NAME >> /etc/wpa_supplicant.conf
	ENTER_SSID_PASSPHRASE
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
iw wlan0 link
dhclient wlan0
```
