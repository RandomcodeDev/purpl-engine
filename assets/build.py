import argparse
import os
import platform
import shutil

from os import PathLike
from subprocess import Popen, PIPE
from types import *
from typing import *

if platform.system() == "Windows":
    exe_ext = ".exe"
else:
    exe_ext = ""

assets_dir = ""
output_dir = ""
tools_dir = ""
repo_tools_dir = ""
repo_tools_native_dir = ""

glslc = ""
meshtool = ""
texturetool = ""
msdf_atlas_gen = ""

verbose = False
rebuild = False
purge = False


def main():
    def parse_arguments():
        parser = argparse.ArgumentParser(description="Your program description here.")
        parser.add_argument(
            "-a",
            "--assets-dir",
            default=os.path.abspath(os.path.dirname(__file__)),
            help="Path to the assets directory (default: current script directory)",
        )
        parser.add_argument(
            "-o",
            "--output-dir",
            default=os.path.join(os.path.abspath(os.path.dirname(__file__)), "out"),
            help="Path to the output directory (default: assets_dir/out)",
        )
        parser.add_argument(
            "-R",
            "--repo-tools-dir",
            default=os.path.abspath(
                os.path.join(os.path.dirname(__file__), "..", "tools")
            ),
            help="Path to the Purpl repository's tools (default: tools folder in the parent directory of the script directory)",
        )
        parser.add_argument(
            "-t",
            "--tools-dir",
            default=os.getenv("PURPL_TOOLS_DIR"),
            help="Path to the tools directory (default: value of PURPL_TOOLS_DIR environment variable)",
        )
        parser.add_argument(
            "-v",
            "--verbose",
            action="store_true",
            default=False,
            help="Whether to print tool output (default: false)",
        )
        parser.add_argument(
            "-r",
            "--rebuild",
            action="store_true",
            default=False,
            help="Whether to rebuild assets (default: false)",
        )
        parser.add_argument(
            "-p",
            "--purge",
            action="store_true",
            default=False,
            help="Whether to purge the output directory (default: false)",
        )

        return parser.parse_args()

    def build_asset(function: FunctionType, src: PathLike, dest: PathLike):
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        dest_exists = os.access(dest, os.F_OK)
        if dest_exists:
            src_stat = os.stat(src)
            dest_stat = os.stat(dest)
            newer = src_stat.st_mtime > dest_stat.st_mtime
        else:
            newer = False
        if rebuild or (newer and dest_exists) or not dest_exists:
            command = function(src, dest)
            print(f"{command} (newer: {newer}, dest_exists: {dest_exists})")
            process = Popen(command, stdout=PIPE, stderr=PIPE)
            stdout, stderr = process.communicate()
            stdout = bytes.decode(stdout, encoding="utf-8")
            stderr = bytes.decode(stderr, encoding="utf-8")
            if verbose:
                print(f"Output:\n{stdout}\n{stderr}")
            if process.returncode != 0:
                print(
                    f"building {src} failed: exit code {process.returncode}"
                )
                exit(1)
        else:
            print(f"Skipping {dest} (newer: {newer}, dest_exists: {dest_exists})")

    args = parse_arguments()
    assets_dir = args.assets_dir
    output_dir = args.output_dir
    tools_dir = args.tools_dir
    repo_tools_dir = args.repo_tools_dir
    repo_tools_native_dir = os.path.join(
        repo_tools_dir,
        platform.system().lower(),
        platform.machine().replace("AMD64", "x64"),
    )
    if tools_dir == None or repo_tools_dir == None:
        print("One or more tool directories not defined, can't continue")
        exit(1)
    verbose = args.verbose
    rebuild = args.rebuild
    purge = args.purge

    print(
        f"{'(Re)b' if rebuild else 'B'}uilding assets in {assets_dir} with tools in {tools_dir}, {repo_tools_dir}, and {repo_tools_native_dir}"
    )

    glslc = os.path.join(repo_tools_native_dir, "glslc" + exe_ext)
    meshtool = os.path.join(tools_dir, "meshtool" + exe_ext)
    texturetool = os.path.join(tools_dir, "texturetool" + exe_ext)
    msdf_atlas_gen = os.path.join(repo_tools_native_dir, "msdf-atlas-gen" + exe_ext)

    print(
        f"{'(Re)b' if rebuild else 'B'}uilding assets in {assets_dir} with tools in {tools_dir} and {repo_tools_dir}"
    )

    if purge:
        print(f"Purging {output_dir}")
        shutil.rmtree(output_dir)

    for root, _, files in os.walk(os.path.join(assets_dir, "models")):
        for file in files:
            build_asset(
                lambda src, dest: [meshtool, "to", src, dest],
                os.path.join(root, file),
                os.path.join(
                    root.replace(assets_dir, output_dir),
                    os.path.splitext(file)[0] + ".pmdl",
                ),
            )

    for root, _, files in os.walk(os.path.join(assets_dir, "textures")):
        for file in files:
            build_asset(
                lambda src, dest: [texturetool, "to", src, dest],
                os.path.join(root, file),
                os.path.join(
                    root.replace(assets_dir, output_dir),
                    os.path.splitext(file)[0] + ".ptex",
                ),
            )

    for root, _, files in os.walk(os.path.join(assets_dir, "shaders", "vulkan")):
        for file in files:
            build_asset(
                lambda src, dest: [glslc, src, "-o", dest],
                os.path.join(root, file),
                os.path.join(root.replace(assets_dir, output_dir), file + ".spv"),
            )

    for root, _, files in os.walk(os.path.join(assets_dir, "fonts")):
        for file in files:
            if os.path.splitext(file)[1] in [".ttf", ".otf"]:
                build_asset(
                    lambda src, dest: [
                        msdf_atlas_gen,
                        "-font",
                        src,
                        "-imageout",
                        dest.replace("json", "png"),
                        "-json",
                        dest,
                        "-charset",
                        os.path.join(assets_dir, "fonts", "charset.txt"),
                    ],
                    os.path.join(root, file),
                    os.path.join(
                        root.replace(assets_dir, output_dir),
                        os.path.splitext(file.lower())[0] + ".json",
                    ),
                )
                build_asset(
                    lambda src, dest: [
                        texturetool,
                        "to",
                        src.replace("json", "png"),
                        dest,
                    ],
                    os.path.join(root, file),
                    os.path.join(
                        root.replace(assets_dir, output_dir),
                        os.path.splitext(file.lower())[0] + ".ptex",
                    ),
                )
                temp_atlas = os.path.join(
                    root.replace(assets_dir, output_dir),
                    os.path.splitext(file.lower())[0] + ".png",
                )
                if os.access(temp_atlas, os.F_OK):
                    os.remove(temp_atlas)


if __name__ == "__main__":
    main()
