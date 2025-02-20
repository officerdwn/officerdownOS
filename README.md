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


VMWare & Physical Hardware
--------------------------
officerdownOS DOES support physical LEGACY hardware and VMWare, 
to run on physical hardware, use a program like rufus to flash 
it as MBR. To run it on VMWare give it around a gigabyte of RAM
and 1 GB of storage (because VMWare doesn't like 0 GB)
and run it.
I have not tested it on Virtualbox, but im sure it would run 
on it.

Screenshot
----------

![ScreenShot](/screenshot.png)

Development Roadmap
-------------------
You can see the Trello roadmap by going to this link:
https://trello.com/b/XhYQSA34/officerdownos-actually
