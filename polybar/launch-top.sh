#!/usr/bin/env bash
# Launches the "top" bar (DisplayPort-2, full info) and the "secondary"
# bar (HDMI-A-0, just workspaces + time) from the cuts theme. No bottom
# bar. Separate from the repo's own launch.sh, which needs a --theme
# flag and, for --cuts, starts both top and bottom bars on one monitor.

pkill -x polybar

# The netspark sparkline is a compiled C helper; the binary isn't tracked
# in git, so (re)build it when it's missing or older than its source.
scripts="$HOME/.config/polybar/cuts/scripts"
if [ "$scripts/netspark.c" -nt "$scripts/netspark" ]; then
    cc -O2 -o "$scripts/netspark" "$scripts/netspark.c" -lm
fi

while pgrep -u "$UID" -x polybar >/dev/null; do
    sleep 1
done

polybar -q top -c "$HOME/.config/polybar/cuts/config.ini" &
polybar -q secondary -c "$HOME/.config/polybar/cuts/config.ini" &
