#!/bin/sh

./tutte --input-file $1 | diff --ignore-all-space - $2
