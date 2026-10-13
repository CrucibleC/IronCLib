import os
import re
import subprocess
import shutil
import sys

# -------------------------------------------------
# Anchor to script location
# -------------------------------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

BUILD_DIR = os.path.join(BASE_DIR, "build", "gcc_single")

# Detect platform-specific executable name
if os.name == "nt":
    EXE_NAME = "hammer_ironclib.exe"
else:
    EXE_NAME = "hammer_ironclib"

EXE_PATH = os.path.join(BUILD_DIR, EXE_NAME)

# -------------------------------------------------
# Optional clean
# -------------------------------------------------
def clean():
    shutil.rmtree(BUILD_DIR, ignore_errors=True)

clean()

os.makedirs(BUILD_DIR, exist_ok=True)

# -------------------------------------------------
# Choose generator (fallback-friendly)
# -------------------------------------------------
def get_cmake_generator():
    # Prefer Ninja if available
    try:
        subprocess.run(["ninja", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return "Ninja"
    except FileNotFoundError:
        pass

    # Windows fallback
    if os.name == "nt":
        return "MinGW Makefiles"

    # Linux/macOS fallback
    return "Unix Makefiles"

GENERATOR = get_cmake_generator()

# -------------------------------------------------
# Find gcc (same rules as run_all.py)
# -------------------------------------------------
def find_gcc():
    # Prefer plain "gcc", otherwise the highest versioned "gcc-N" on PATH.
    # On macOS plain "gcc" is Apple clang in disguise, so only a versioned
    # (e.g. Homebrew "gcc-14") gcc counts there.
    if shutil.which("gcc") and sys.platform != "darwin":
        return "gcc"

    best = None
    best_version = -1
    pattern = re.compile(r"^gcc-(\d+)(\.exe)?$")

    for path_dir in os.environ.get("PATH", "").split(os.pathsep):
        if not os.path.isdir(path_dir):
            continue
        for entry in os.listdir(path_dir):
            match = pattern.match(entry)
            if match and int(match.group(1)) > best_version:
                best_version = int(match.group(1))
                best = os.path.join(path_dir, entry)

    return best

GCC = find_gcc()
if not GCC:
    print("gcc not found on PATH")
    sys.exit(1)

print(f"Using gcc: {GCC}")

# -------------------------------------------------
# Configure CMake
# -------------------------------------------------
print("Configuring CMake...")

subprocess.check_call([
    "cmake",
    "-S", BASE_DIR,
    "-B", BUILD_DIR,
    "-G", GENERATOR,
    f"-DCMAKE_C_COMPILER={GCC}",
    "-DCOMPILER_NAME=gcc", # makes CMake fail if the compiler is not really gcc
    "-DC_STD=c11",
    "-DOPT_LEVEL=-O2",
])

# -------------------------------------------------
# Build
# -------------------------------------------------
print("Building...")

subprocess.check_call([
    "cmake",
    "--build", BUILD_DIR
])

# -------------------------------------------------
# Run
# -------------------------------------------------
print("Running...\n")

if not os.path.exists(EXE_PATH):
    raise FileNotFoundError(f"Executable not found: {EXE_PATH}")

# Use list form for better cross-platform handling
result = subprocess.run([EXE_PATH])

print(f"\nExit code: {result.returncode}")

# Propagate the test result so callers (and CI) see failures
sys.exit(result.returncode)
