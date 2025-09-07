#!/bin/bash
# make_playlist.sh
# generates playlist.txt with absolute paths of all audio files in current directory (and subdirectories)

# name of playlist file
playlist="playlist.txt"

# extensions to include (add more if you want)
exts="mp3|wav|flac|ogg|m4a|mp4|webm|opus"

# clear old playlist
> "$playlist"

# find all files with those extensions, sorted, write absolute paths
find "$(pwd)" -type f | grep -Ei "\.($exts)$" | sort >> "$playlist"

echo "playlist written to $playlist with $(wc -l < "$playlist") tracks."

