#!/bin/sh
# Extreme Tux Racer launcher for Trimui (stock OS / GarlicOS PORTS section)
progdir="$(cd "$(dirname "$0")" && pwd)/ExtremeTuxRacer"
cd "$progdir" || exit 1
export HOME="$progdir"
export LD_LIBRARY_PATH="/usr/trimui/lib:/usr/lib:$LD_LIBRARY_PATH"
export LIBGL_NOBANNER=1
[ -f "$progdir/debug" ] && export ETR_DEBUG_INPUT=1
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
./etr > "$progdir/etr.log" 2>&1
sync
