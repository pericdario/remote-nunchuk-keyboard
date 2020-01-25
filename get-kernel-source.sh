#!/bin/bash

apt-get update
apt-get install bc

cd ~

REV=`zcat /usr/share/doc/raspberrypi-bootloader/changelog.Debian.gz | grep '* firmware as of' | head -n 1 | sed  -e 's/\  \*\ firmware as of \(.*\)$/\1/'`

rm -rf rasp-tmp
mkdir -p rasp-tmp
mkdir -p rasp-tmp/linux

wget https://raw.github.com/raspberrypi/firmware/$REV/extra/git_hash -O rasp-tmp/git_hash
wget https://raw.github.com/raspberrypi/firmware/$REV/extra/Module7.symvers -O rasp-tmp/Module.symvers

SOURCEHASH=`cat rasp-tmp/git_hash` 

wget https://github.com/raspberrypi/linux/tarball/$SOURCEHASH -O rasp-tmp/linux.tar.gz

cd  rasp-tmp
tar -xzf linux.tar.gz

OSVERSION=`uname -r`

rm -rf /usr/src/linux-source-$OSVERSION/

mv raspberrypi-linux* /usr/src/linux-source-$OSVERSION
rm -f /lib/modules/$OSVERSION/build
ln -s /usr/src/linux-source-$OSVERSION /lib/modules/$OSVERSION/build

cp Module.symvers /usr/src/linux-source-$OSVERSION/

modprobe configs
zcat /proc/config.gz > /usr/src/linux-source-$OSVERSION/.config

cd /usr/src/linux-source-$OSVERSION/
make -j4 oldconfig
make -j4 prepare
make -j4 scripts
chmod -Rf 777 *
cd ~
rm -rf rasp-tmp
