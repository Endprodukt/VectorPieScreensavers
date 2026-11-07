#!/bin/sh
# make_vps.sh <folder> - a screensaver as a VectorPie package:
# build/<id>-<version>.vps, e.g.  sh tools/make_vps.sh neondrive
#
# Run from the repository's folder after the Pi build (BUILD.md). The
# package holds the Pi program and game.ini (kind = screensaver, so vp-pack
# writes a .vps); the cabinet installs it in SETTINGS -> SCREENSAVERS ->
# INSTALL. Made like Vector Racer's tools/make_vpk.sh.
set -eu
[ $# -eq 1 ] || { echo "usage: make_vps.sh <folder>" >&2; exit 1; }
root=$(cd "$(dirname "$0")/.." && pwd)
saver=$1
ini="$root/$saver/package/game.ini"
[ -f "$ini" ] || { echo "make_vps: no $ini" >&2; exit 1; }
exe=$(sed -n 's/^executable *= *\([^ ;]*\).*/\1/p' "$ini" | tr -d '\r')
id=$(sed -n 's/^id *= *\([^ ;]*\).*/\1/p' "$ini" | tr -d '\r')
prog="$root/build/pi/$saver/$exe"
stage="$root/build/pkg/$id"
[ -f "$prog" ] || { echo "make_vps: no Pi build $prog" >&2; exit 1; }
rm -rf "$root/build/pkg"
mkdir -p "$stage"
cp "$prog" "$ini" "$stage/"
sh "$root/vendor/VectorPie/sdk/tools/vp-pack/vp-pack.sh" "$stage" "$root/build"

# Built on Windows the program in the archive is not marked as one: the file
# system there has no such mark for vp-pack's chmod to set. Mark it here.
vps=$(ls -t "$root"/build/$id-*.vps | head -n 1)
python - "$vps" "$id/$exe" <<'PY'
import io, sys, tarfile
path, name = sys.argv[1], sys.argv[2]
out = io.BytesIO()
with tarfile.open(path, "r:gz") as old, tarfile.open(fileobj=out, mode="w:gz") as new:
    for member in old.getmembers():
        if member.name == name:
            member.mode = 0o755
        new.addfile(member, old.extractfile(member) if member.isfile() else None)
open(path, "wb").write(out.getvalue())
PY
echo "make_vps: $vps"
