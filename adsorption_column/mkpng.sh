#!/bin/bash
for f in *.svg; do inkscape -d 150 "$f" -e "$(basename "$f" ".svg").png"; done
