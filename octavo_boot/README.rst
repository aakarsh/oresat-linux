Octavo USB-eMMC Flasher
=======================

This software allows a host computer to flash to the Octavo's eMMC via USB. It
is based on the guide here: 
https://octavosystems.com/app_notes/programming-emmc-with-usb/

However, python is used instead of `dhcpd` and `xinetd` so it could be run as
a standalone script instead of needing to setup multiple services

Setup
-----

Install dependencies

.. code-block::

   $ sudo apt install ifconfig unzip curl git
   $ sudo pip3 install -r requirements.txt

Clone the oresat-linux repo

.. code-block::

   $ git clone https://github.com/oresat/oresat-linux
   $ cd oresat-linux/octavo-boot/

Download the project files
    
.. code-block::

    $ curl -O https://octavosystems.com/octavosystems.com/wp-content/uploads/2019/02/Programming-eMMC-with-USB-for-OSD335x.zip

Unzip and rename folder

.. code-block::

    $ unzip Programming-eMMC-with-USB-for-OSD335x.zip
    $ mv eMMC\ Flasher\ Project/ tftpboot

Boot Octavo via USB as USB mass storage
---------------------------------------

- Ensure Octavo board is in SD card boot mode but there is **NO** SD car inserted.
- Plug in Octavo board and see it appear as network interface. For example, it can 
  appears as `usb0`

.. code-block::

    $ ifconfig usb0
    usb0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
            inet6 fe80::7d10:b6e9:56af:566  prefixlen 64  scopeid 0x20<link>
            ether ca:03:ae:3a:5b:b8  txqueuelen 1000  (Ethernet)
            RX packets 2  bytes 784 (784.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 8  bytes 1634 (1.5 KiB)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

- Run the boot script with the network interface and absolute path to the `tfptboot`
  directory. **NOTE:** this should be done within 10~15 seconds of the Octavo board
  being plugged in.

.. code-block::

    $ sudo python3 octavo-_usb_boot.py usb0 /home/pi/oresat-linux/utils/octavo-boot/tftpboot
    INFO(tftpd): Serving TFTP requests on usb0/192.168.0.1:69 in /home/pi/oresat-linux/utils/octavo-boot/tftpboot
    INFO(octavo-usb-boot): Handling 'AM335x ROM' BOOTP packet
    INFO(bootpd): Offering to boot client 192.168.0.2
    INFO(bootpd): Booting client 192.168.0.2 with file b'u-boot-spl-restore.bin'
    INFO(tftpd): Serving file u-boot-spl-restore.bin to host 192.168.0.2...
    INFO(tftpd): Transfer of file u-boot-spl-restore.bin completed.
    INFO(octavo-usb-boot): Handling 'AM335x U-Boot SPL' BOOTP packet
    ...

- After a minute a USB mass storage device appear on the host system. After
  this, it is safe to kill the above script

 .. code-block::

    # dmesg output
    [177014.042105] usb-storage 1-1.1:1.0: USB Mass Storage device detected
    [177014.044351] usb-storage 1-1.1:1.0: Quirks match for vid 0525 pid a4a5: 10000
    [177014.044521] scsi host0: usb-storage 1-1.1:1.0
    [177015.089132] scsi 0:0:0:0: Direct-Access     Linux    File-Stor Gadget 0414 PQ: 0 ANSI: 2
    [177015.089789] scsi 0:0:0:0: Attached scsi generic sg0 type 0
    [177015.092060] sd 0:0:0:0: [sda] 30777344 512-byte logical blocks: (15.8 GB/14.7 GiB)
    [177015.199190] sd 0:0:0:0: [sda] Write Protect is off
    [177015.199206] sd 0:0:0:0: [sda] Mode Sense: 0f 00 00 00
    [177015.307238] sd 0:0:0:0: [sda] Write cache: enabled, read cache: enabled, doesn't support DPO or FUA
    [177015.557922]  sda: sda1
    [177015.775279] sd 0:0:0:0: [sda] Attached SCSI removable disk
    ...

 .. code-block::

    # sudo fdisk -l output
    Disk /dev/sda: 14.7 GiB, 15758000128 bytes, 30777344 sectors
    Disk model: File-Stor Gadget
    Units: sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disklabel type: dos
    Disk identifier: 0xcbad22ad

Download OS image
-----------------

OreSat images can be found at https://debian.oresat.org/images/

dd it onto the eMMC
-------------------

.. code-block::

    $ sudo dd status=progress if=oresat-startracker-2020-12-19.img of=/dev/sda

- Remove power, change the boot jumper to boot from eMMC, and power up. Board
  should boot from eMMC
