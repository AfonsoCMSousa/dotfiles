#!/usr/bin/env bash
# Keeps one Anki instance running in the background, so the polybar Anki
# module (which talks to AnkiConnect) works without the app being open.
#
# Its main window is parked in the i3 scratchpad by a for_window rule in
# i3/config; open it with anki-show (what the rofi Anki entry runs). If you
# quit Anki, it's started again, hidden, a few seconds later.
#
# Anki locks its collection exclusively and is single-instance, so this is
# safer than reading the collection directly: there is only ever one
# process touching it. If you opened Anki yourself, this just waits.

exec 9>"${XDG_RUNTIME_DIR:-/tmp}/anki-daemon.lock"
flock -n 9 || exit 0 # already running

running() { pgrep -f '^/usr/bin/python3 /usr/bin/anki( |$)' >/dev/null; }

while true; do
    if ! running; then
        # 9>&- so Anki doesn't inherit (and hold) the lock
        anki 9>&- >/dev/null 2>&1
    fi
    sleep 5
done
