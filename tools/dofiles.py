#!/usr/bin/env python3

import os
import sys

from subprocess import Popen, PIPE

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <command> [arguments]")
        sys.exit()

    rootpath = os.path.abspath(os.path.basename(sys.argv[0]) + os.sep + "..")
    for root, dirs, files in os.walk(rootpath):
       for file in files:
            if not any(dir in root for dir in [".cache", ".idea", ".vscode", "build", "deps", "tools"]) and os.path.splitext(file)[1] in [".c", ".cpp", ".h"]:
                args = sys.argv[1:] + [root + os.sep + file]
                p = Popen(args, stdout=PIPE, stderr=PIPE)
                stdout, stderr = p.communicate()
                print(f"{args}\nstdout:\n{bytes.decode(stdout, encoding='utf-8')}\nstderr{bytes.decode(stderr, encoding='utf-8')}")

if __name__ == "__main__":
    main()
