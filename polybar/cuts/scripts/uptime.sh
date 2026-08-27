#!/usr/bin/env bash
# Fixed-width uptime string for polybar, e.g. "3d 07h30m" or "0d 07h30m"

read -r up _ < /proc/uptime
up=${up%.*}

days=$(( up / 86400 ))
hours=$(( (up % 86400) / 3600 ))
mins=$(( (up % 3600) / 60 ))

printf "%dd %02dh%02dm" "$days" "$hours" "$mins"
