officerdownOS
==============
credit to fdipilla for the base.


My biggest programming dream, coming true.
Not much, but cool.
I will try to update this as much as i can, because it is my coolest project.
Please make an issue with suggetons for what I should add. Its pretty barebones right now.
I don't plan to make a GUI.

Dependencies
------------

* nasm
* gcc
* ld
* qemu
* make


Compile & Run
-------------

First you need to compile the kernel with `make`, now you are ready to run it with `make run`
Clear your workspace with `make clean`


VMware & Physical Hardware
--------------------------
officerdownOS DOES support physical LEGACY hardware and VMware, 
to run on physical hardware, use a program like rufus to flash 
it as MBR. To run it on VMware give it around a gigabyte of RAM
and 1 GB of storage (because VMware doesn't like 0 GB)
and run it.
I have not tested it on Virtualbox, but im sure it would run 
on it.
PSA: With the v0.1.3 Update, at least with my testing, 
officerdownOS will not boot on legacy hardware, it still 
works on VMWare though. I will hopefully release a patched
iso tomorrow. Meanwhile, use v0.1.2 on lagacy
hardware.

Screenshot
----------

![ScreenShot](/screenshot.png)

Development Roadmap
-------------------
You can see the Trello roadmap by going to this link:
https://trello.com/b/XhYQSA34/officerdownos-actually
