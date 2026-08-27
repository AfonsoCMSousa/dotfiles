#!/usr/bin/env bash
# Scrolling random hex bytes - a little cyberpunk ticker for the empty
# space on the secondary bar. Mostly green, with an occasional red byte.

width=28
buf=()

rand_byte() {
    local hex
    hex=$(printf '%02X' $((RANDOM % 256)))
    if (( RANDOM % 10 == 0 )); then
        echo "%{F#ff0048}${hex}%{F-}"
    else
        echo "%{F#c9f299}${hex}%{F-}"
    fi
}

for ((i = 0; i < width; i++)); do
    buf+=("$(rand_byte)")
done

while true; do
    echo "${buf[*]}"
    sleep 0.2
    buf=("${buf[@]:1}")
    buf+=("$(rand_byte)")
done
