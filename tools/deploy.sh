#!/bin/sh
# deploy.sh <folder> - install a screensaver's Pi build on the cabinet:
# /persistent/screensavers/<id> (where the menu's SETTINGS -> SCREENSAVERS
# -> INSTALL puts a .vps; the same layout: the program, game.ini, save/).
# Choose it there in SETTINGS -> SCREENSAVERS -> SELECT.
#   sh tools/deploy.sh neondrive
# Run from the repository's folder, in Git Bash, after the Pi build
# (BUILD.md). The same key and cabinet as Vector Racer's tools/deploy.sh.
set -eu
[ $# -eq 1 ] || { echo "usage: deploy.sh <folder>" >&2; exit 1; }
saver=$1
ini="$saver/package/game.ini"
[ -f "$ini" ] || { echo "deploy: no $ini (run from the repository's folder)" >&2; exit 1; }
exe=$(sed -n 's/^executable *= *\([^ ;]*\).*/\1/p' "$ini" | tr -d '\r')
id=$(sed -n 's/^id *= *\([^ ;]*\).*/\1/p' "$ini" | tr -d '\r')
prog="build/pi/$saver/$exe"
[ -f "$prog" ] || { echo "deploy: no Pi build $prog" >&2; exit 1; }
K="-i $HOME/.ssh/id_ed25519_vectorpie -o ConnectTimeout=8 -o BatchMode=yes"
H=pi@192.168.0.122
D=/persistent/screensavers/$id
scp -q $K "$prog" "$ini" $H:/tmp/
ssh $K $H "set -e
sudo -n mkdir -p $D/save
sudo -n cp /tmp/$exe $D/$exe.new
sudo -n cp /tmp/game.ini $D/
sudo -n chown -R pi:pi $D
chmod 755 $D/$exe.new
mv -f $D/$exe.new $D/$exe   # also while it runs (a new file; the running one lives on)
rm -f /tmp/$exe /tmp/game.ini
sync
grep -E '^(name|version)' $D/game.ini"
echo "deploy: installed in $D"
