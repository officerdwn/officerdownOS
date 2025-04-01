!! AS OF 4/1/2025 officerdownOS is deprecated, it has discontinued development!!
================================================================================

![officerdownOS logo](/logo.png)

officerdownOS
==============
credit to fdipilla for the base.


My biggest programming dream, coming true.
Not much, but cool.
I will try to update this as much as i can, because it is my coolest project.
Please make an issue with suggetons for what I should add. Its pretty barebones right now.
I have previously stated that I wont add a GUI, but I am considering adding one.

Dependencies
------------

* nasm
* gcc
* ld
* qemu
* make
* xorriso
* grub-mkrescue


Compile & Run
-------------

First you need to compile the kernel with `make`, now you are ready to run it with `make run`.
Clear your workspace with `make clean`. Make an ISO with `make iso`.

VMware & Physical Hardware
--------------------------
officerdownOS DOES support physical LEGACY hardware and VMware, 
to run on physical hardware, use a program like rufus to flash 
it as MBR. To run it on VMware give it around a gigabyte of RAM
and 1 GB of storage (because VMware doesn't like 0 GB)
and run it.
I have not tested it on Virtualbox, but im sure it would run 
on it.
NOTE: Use the prebuilt ISO for booting on physical hardware and VMWare.
(or build it and make a GRUB image)
PSA: Physical Hardware booting has had issues with v0.1.4, Just use a VM.

Versioning
----------

![officerdownOS versioning](/versioning.png)


Development Roadmap
-------------------
You can see the Trello roadmap by going to this link:
https://trello.com/b/XhYQSA34/officerdownos-actually

Screenshot
----------

![ScreenShot](/screenshot.png)



