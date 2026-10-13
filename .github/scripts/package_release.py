"""
Builds the IronCLib release archives.

Usage:
    python .github/scripts/package_release.py <version> <output_dir> [--check-only]

    <version>     MAJOR.MINOR.PATCH, e.g. 1.0.3 (a leading "v", as in the tag, is accepted)
    <output_dir>  where ironclib-<version>.zip and ironclib-<version>.tar.gz are written
    --check-only  only verify that the version matches the repository, build nothing

The version must match IC_VERSION_STRING (and the MAJOR/MINOR/PATCH macros) in
ironclib/ic_version.h, and the version in the README title, so a release can never
be published with a mismatched number.

Archive layout (premade headers are intentionally not included):
    ironclib-<version>/
        include/ironclib/*.h   (so #include "ironclib/ic.h" works with -I include)
        LICENSE
        NOTICE
        AUTHORS
        README.md
"""

import os
import re
import shutil
import sys
import tarfile
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
VERSION_HEADER = os.path.join(ROOT, "ironclib", "ic_version.h")
README = os.path.join(ROOT, "README.md")
TOP_LEVEL_FILES = ["LICENSE", "NOTICE", "AUTHORS", "README.md"]


def fail(message):
    print(f"ERROR: {message}")
    sys.exit(1)


def read_header_version():
    with open(VERSION_HEADER, encoding="utf-8") as f:
        text = f.read()

    def macro(name, pattern):
        match = re.search(rf"#define\s+{name}\s+{pattern}", text)
        if not match:
            fail(f"{name} not found in {VERSION_HEADER}")
        return match.group(1)

    major = macro("IC_VERSION_MAJOR", r"(\d+)")
    minor = macro("IC_VERSION_MINOR", r"(\d+)")
    patch = macro("IC_VERSION_PATCH", r"(\d+)")
    string = macro("IC_VERSION_STRING", r'"([^"]*)"')

    if string != f"{major}.{minor}.{patch}":
        fail(f"ic_version.h is inconsistent: IC_VERSION_STRING is {string}, "
             f"but MAJOR/MINOR/PATCH give {major}.{minor}.{patch}")
    return string


def read_readme_version():
    with open(README, encoding="utf-8") as f:
        for line in f:
            match = re.match(r"#\s+IronCLib\s+[vV](\d+\.\d+\.\d+)\s*$", line)
            if match:
                return match.group(1)
    fail("README.md has no '# IronCLib vX.Y.Z' title")


def check_version(version):
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        fail(f"'{version}' is not a MAJOR.MINOR.PATCH version")

    header_version = read_header_version()
    readme_version = read_readme_version()

    if version != header_version:
        fail(f"version {version} does not match IC_VERSION_STRING {header_version} in ironclib/ic_version.h")
    if version != readme_version:
        fail(f"version {version} does not match the README title version {readme_version}")

    print(f"Version {version} matches ic_version.h and README.md")


def build_archives(version, output_dir):
    name = f"ironclib-{version}"
    staging_root = os.path.join(output_dir, "staging")
    package_dir = os.path.join(staging_root, name)

    shutil.rmtree(staging_root, ignore_errors=True)
    include_dir = os.path.join(package_dir, "include", "ironclib")
    os.makedirs(include_dir)

    headers = sorted(f for f in os.listdir(os.path.join(ROOT, "ironclib")) if f.endswith(".h"))
    if not headers:
        fail("no headers found in ironclib/")
    for header in headers:
        shutil.copy2(os.path.join(ROOT, "ironclib", header), include_dir)

    for file in TOP_LEVEL_FILES:
        source = os.path.join(ROOT, file)
        if not os.path.isfile(source):
            fail(f"{file} is missing from the repository root")
        shutil.copy2(source, package_dir)

    # Both archives contain the single top-level folder ironclib-<version>/
    zip_path = os.path.join(output_dir, f"{name}.zip")
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as archive:
        for folder, _, files in os.walk(package_dir):
            for file in sorted(files):
                path = os.path.join(folder, file)
                archive.write(path, os.path.relpath(path, staging_root))

    tar_path = os.path.join(output_dir, f"{name}.tar.gz")
    with tarfile.open(tar_path, "w:gz") as archive:
        archive.add(package_dir, arcname=name)

    shutil.rmtree(staging_root)

    print(f"Packaged {len(headers)} headers and {', '.join(TOP_LEVEL_FILES)}:")
    print(f"  {zip_path}")
    print(f"  {tar_path}")


def main():
    args = [a for a in sys.argv[1:] if a != "--check-only"]
    check_only = "--check-only" in sys.argv[1:]

    if len(args) != (1 if check_only else 2):
        print(__doc__)
        sys.exit(2)

    version = args[0][1:] if args[0].startswith(("v", "V")) else args[0]
    check_version(version)

    if not check_only:
        output_dir = os.path.abspath(args[1])
        os.makedirs(output_dir, exist_ok=True)
        build_archives(version, output_dir)


if __name__ == "__main__":
    main()
