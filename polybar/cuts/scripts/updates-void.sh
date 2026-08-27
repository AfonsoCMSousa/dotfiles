#!/usr/bin/env bash
# Void Linux (xbps) update count for polybar.
# Syncs repodata and lists pending updates without installing anything.

count=$(xbps-install -Sun 2>/dev/null | wc -l)
padded=$(printf "%3d" "$count")

if [ "$count" -gt 0 ]; then
    echo "%{F#ff0048}${padded}%{F-}"
else
    echo "%{F#c9f299}${padded}%{F-}"
fi
