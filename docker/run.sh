#!/bin/sh

set -e

log() {
	printf "[%11s] %s\n" "$(cut -d' ' -f1 /proc/uptime)" "$*"
}

for f in /tmp/data/data.zip /tmp/data/options.txt;do
	test -e "$f" || {
		echo "There are no $f"
		exit 1
	}
done

log start
mkdir tmp
cd ~/tmp
cp /tmp/data/options.txt .
unzip -q /tmp/data/data.zip
log unzipped
exec ~/serv . 80
