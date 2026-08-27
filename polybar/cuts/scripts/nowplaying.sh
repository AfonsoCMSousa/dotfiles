#!/usr/bin/env bash
# "Now playing" widget via playerctl (MPRIS). Prefers Spotify if it's
# running, otherwise falls back to whatever else is actively playing
# (e.g. a YouTube tab in Brave).

players=$(playerctl -l 2>/dev/null)

if [ -z "$players" ]; then
    echo "%{F#5a6570} Nothing playing%{F-}"
    exit 0
fi

player=$(echo "$players" | grep -m1 '^spotify')

if [ -z "$player" ]; then
    while read -r p; do
        if [ "$(playerctl -p "$p" status 2>/dev/null)" = "Playing" ]; then
            player="$p"
            break
        fi
    done <<< "$players"
fi

[ -z "$player" ] && player=$(echo "$players" | head -1)

status=$(playerctl -p "$player" status 2>/dev/null)
info=$(playerctl -p "$player" metadata --format '{{ artist }} - {{ title }}' 2>/dev/null)
[ -z "$info" ] && info="Nothing playing"

maxlen=40
if [ "${#info}" -gt "$maxlen" ]; then
    info="${info:0:$((maxlen - 3))}..."
fi

if [ "$status" = "Playing" ]; then
    echo "%{F#c9f299}%{F-} ${info}"
elif [ "$status" = "Paused" ]; then
    echo "%{F#5a6570}%{F-} ${info}"
else
    echo "%{F#5a6570}%{F-} ${info}"
fi
