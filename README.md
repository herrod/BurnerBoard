BurnerBoard
===========

Code that powers the 2013 Burning Man Burner Board

Copyright 2013

Some notes for the new Beaglebone node.

After getting the board up, clone this repo onto the board.

From
cd BurnerBoard/beaglebone
cp *.dts /lib/firmware/

cd /boot/uboot/
vi uEnv.txt

Disable HDMI

Then 
cd ~/BurnerBoard/experiments/node_modules/burnerboard
node burnerboard.js
