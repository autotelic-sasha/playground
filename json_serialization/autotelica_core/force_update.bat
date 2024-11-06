attrib -h .original_dot_git
move .original_dot_git .git
git fetch --all
git reset --hard origin/main
move .git .original_dot_git
attrib +h .original_dot_git
