#!/usr/bin/sh
mv .original_dot_git .git
git fetch --all
git reset --hard origin/main
mv .git .original_dot_git
