#!/usr/bin/env bash
# Windows+D style "show desktop" toggle for i3 - hides/restores BOTH
# monitors together. Remembers what each output was showing, switches
# both to an empty peek workspace, and restores both on the next press.

STATE_FILE="$HOME/.cache/i3-desktop-peek-state"
OUT_1="DisplayPort-2"
PEEK_1="0"
OUT_2="HDMI-A-0"
PEEK_2="00"

if [ -f "$STATE_FILE" ]; then
    # Currently peeking -> restore both outputs to what they had before
    while IFS='|' read -r _output name; do
        [ -n "$name" ] && i3-msg "workspace $name" >/dev/null
    done < "$STATE_FILE"
    rm -f "$STATE_FILE"
else
    # Not peeking -> remember what's currently shown on each output, then
    # switch both to their empty peek workspace
    i3-msg -t get_workspaces | python3 -c "
import json, sys
for w in json.load(sys.stdin):
    if w['visible']:
        print(f\"{w['output']}|{w['name']}\")
" > "$STATE_FILE"

    i3-msg "focus output $OUT_1; workspace $PEEK_1" >/dev/null
    i3-msg "focus output $OUT_2; workspace $PEEK_2" >/dev/null
fi
