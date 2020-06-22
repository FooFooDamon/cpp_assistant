import os

flags = [
    "-Wall"
    , "-std=c++11"
    , "-x", "c++"
]

for inc_dir in os.popen("echo | $(g++ --print-prog-name=cc1plus) -v 2>&1 | grep '/usr/.*include' | grep -v '^ignoring' | sed 's/^[ ]*//g'").read().split("\n"):
    if "" != inc_dir:
        flags.append("-I")
        flags.append(inc_dir)

SOURCE_EXTENSIONS = [ ".c", ".C", ".cc", ".cpp", ".cxx", ".c++" ]

def FlagsForFile(filename, **kwargs):
    return { "flags": flags, "do_cache": True }

