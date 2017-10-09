#!/bin/bash

DIR=$1

cp -f patch102_fix.xai $DIR/app/PCSG00881/patch102.xai
cp -f patch102_fix_ch.xai $DIR/app/PCSH00297/patch102.xai

#./xaiPatch $DIR/app/PCSG00881/rootast.xai custom/mp4102t2.bin script/mp4102t2.bin

#./patchEboot.py $DIR/app/PCSG00881/eboot.bin
#./patchEboot.py $DIR/app/PCSG00881/mai_moe/eboot_origin.bin

./patchEboot.py $DIR/app/PCSH00297/eboot.bin
./patchEboot.py $DIR/app/PCSH00297/mai_moe/eboot_origin.bin


umount $DIR -v
