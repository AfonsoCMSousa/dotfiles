#!/usr/bin/env bash
# Void Linux (xbps) update count for polybar.
# Syncs repodata and lists pending updates without installing anything.

# wallpaper palette from ~/.config/theme/theme (magenta defaults if missing)
. "${XDG_CACHE_HOME:-$HOME/.cache}/theme/colors.sh" 2>/dev/null

count=$(xbps-install -Sun 2>/dev/null | wc -l)
padded=$(printf "%3d" "$count")

if [ "$count" -gt 0 ]; then
    echo "%{F${accent:-#ff0048}}${padded}%{F-}"
else
    echo "%{F${secondary:-#c9f299}}${padded}%{F-}"
fi
