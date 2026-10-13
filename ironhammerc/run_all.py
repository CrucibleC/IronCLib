import subprocess
import os
import shutil
import re
import json
import sys

# -----------------------------
# Paths (portable)
# -----------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_ROOT = os.path.join(BASE_DIR, "build")

IS_WINDOWS = os.name == "nt"
IS_MACOS = sys.platform == "darwin"
EXE_NAME = "hammer_ironclib.exe" if IS_WINDOWS else "hammer_ironclib"

# Optional sanitizers, passed to CMakeLists.txt (disabled unless env var is set).
# e.g. SANITIZERS=address,undefined or SANITIZERS=thread (gcc/clang); MSVC supports only "address".
# Not every toolchain ships the runtimes (MinGW gcc/clang mostly do not), so a missing one fails the build.
SANITIZERS = os.environ.get("SANITIZERS", "")

# -----------------------------
# Generator helpers
# -----------------------------
def get_generator(preferred):
    if preferred == "Ninja" and shutil.which("ninja"):
        return "Ninja"

    if IS_WINDOWS:
        if shutil.which("ninja"):
            return "Ninja"
        return "MinGW Makefiles"
    else:
        if shutil.which("ninja"):
            return "Ninja"
        return "Unix Makefiles"


def find_visual_studio_generator():
    # Returns (generator, error):
    #   (name, None)  - Visual Studio found and CMake has a generator for it
    #   (None, None)  - no Visual Studio with C++ tools installed (msvc is skipped)
    #   (None, error) - Visual Studio installed but unusable (msvc counts as a failure, so CI
    #                   cannot silently stop testing msvc)
    # Ask vswhere (ships with every VS install) for the newest VS with C++ tools
    if not IS_WINDOWS:
        return None, None

    program_files = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    vswhere = os.path.join(
        program_files, "Microsoft Visual Studio", "Installer", "vswhere.exe"
    )
    if not os.path.exists(vswhere):
        return None, None

    try:
        output = subprocess.check_output(
            [
                vswhere,
                "-latest",
                "-products", "*",
                "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                "-format", "json",
            ],
            text=True,
        )
        installs = json.loads(output)
    except (subprocess.CalledProcessError, ValueError):
        return None, None

    if not installs:
        return None, None

    # The generator name is "Visual Studio <major> <year>", e.g. "Visual Studio 17 2022" or
    # "Visual Studio 18 2026". vswhere's productLineVersion cannot be used for the year: it is
    # "2022" for VS 2022 but "18" for VS 2026. So ask CMake which generator matches the major version.
    major = installs[0]["installationVersion"].split(".")[0]
    # Exactly "Visual Studio <major> <year>": CMake also lists legacy names with an architecture
    # suffix (e.g. "Visual Studio 15 2017 Win64"), which must not be picked
    name_pattern = re.compile(rf"^Visual Studio {re.escape(major)} \d{{4}}$")

    try:
        capabilities = json.loads(
            subprocess.check_output(["cmake", "-E", "capabilities"], text=True)
        )
        for generator in capabilities.get("generators", []):
            name = generator.get("name", "")
            if name_pattern.match(name):
                return name, None
    except (subprocess.CalledProcessError, OSError, ValueError) as e:
        return None, f"could not query CMake generators ({e})"

    # This CMake does not know this Visual Studio version yet
    return None, f"no CMake generator for Visual Studio {major} (a newer CMake is needed)"


def find_compiler(name):
    # Prefer plain "<name>", otherwise the highest versioned "<name>-N" on PATH.
    # On macOS plain "gcc" is Apple clang in disguise, so only a versioned
    # (e.g. Homebrew "gcc-14") gcc counts there.
    if shutil.which(name) and not (IS_MACOS and name == "gcc"):
        return name

    best = None
    best_version = -1
    pattern = re.compile(rf"^{re.escape(name)}-(\d+)(\.exe)?$")

    for path_dir in os.environ.get("PATH", "").split(os.pathsep):
        if not os.path.isdir(path_dir):
            continue
        for entry in os.listdir(path_dir):
            match = pattern.match(entry)
            if match and int(match.group(1)) > best_version:
                best_version = int(match.group(1))
                best = os.path.join(path_dir, entry)

    return best


# -----------------------------
# Compiler matrix
# -----------------------------
configs = [
    ("gcc", find_compiler("gcc"), "Ninja"),
    ("clang", find_compiler("clang"), "Ninja"),
]

# MSVC is only available on Windows
MSVC_GENERATOR_ERROR = None
if IS_WINDOWS:
    MSVC_GENERATOR, MSVC_GENERATOR_ERROR = find_visual_studio_generator()
    configs.append(("msvc", None, MSVC_GENERATOR))


# -----------------------------
# Test matrix
# -----------------------------
tests = [
    # Debug builds (important for correctness)
    ("c99", "-O0"),
    ("c11", "-O0"),

    # Optimized builds
    ("c99", "-O2"),
    ("c11", "-O2"),
]

# One-off runs for optional backends, so they don't multiply the whole matrix
# (compiler, std, opt, tag, defines)
C11_THREADS = ["IC_USE_C11_THREADS_AND_ATOMICS"]

# MinGW has no C11 <threads.h>, so on Windows the C11 backend is tested with MSVC (VS 2022 17.8+).
# macOS has no <threads.h> at all, so the C11 backend is not tested there.
if IS_WINDOWS:
    extra_tests = [("msvc", "c11", "-O2", "c11threads", C11_THREADS)]
elif IS_MACOS:
    extra_tests = []
else:
    extra_tests = [("clang", "c11", "-O2", "c11threads", C11_THREADS)]

# ThreadSanitizer tracks threads by intercepting pthread_create, but glibc's C11 thrd_create starts
# threads without going through it. TSan then crashes inside its own runtime on the first access
# from such a thread ("SEGV on unknown address", "nested bug in the same thread"), which says nothing
# about IronCLib. The pthread backend, used by every other configuration, is fully covered by TSan.
if "thread" in SANITIZERS.split(","):
    for name, std, opt, tag, defines in extra_tests:
        if tag == "c11threads":
            print(f"{name} {tag} skipped (ThreadSanitizer does not support C11 thrd_create threads)")
    extra_tests = [entry for entry in extra_tests if entry[3] != "c11threads"]


# -----------------------------
# Helpers
# -----------------------------
def run(cmd, cwd=None):
    print("RUN:", " ".join(cmd))
    subprocess.check_call(cmd, cwd=cwd)


def run_exe(exe):
    print("EXEC:", exe)
    result = subprocess.run([exe])
    return result.returncode


# -----------------------------
# Test runner
# -----------------------------
failures = []
successes = []

for compiler_name, compiler, preferred_gen in configs:

    if compiler_name in ("gcc", "clang"):
        if not compiler:
            print(f"{compiler_name} not found, skipping")
            continue
        print(f"Using {compiler_name}: {compiler}")

    if compiler_name == "msvc":
        if not preferred_gen:
            if MSVC_GENERATOR_ERROR:
                # Visual Studio is installed, so skipping would hide that msvc is not tested
                failures.append(("msvc", MSVC_GENERATOR_ERROR))
                print(f"FAIL: msvc ({MSVC_GENERATOR_ERROR})")
            else:
                print("msvc skipped (Visual Studio not available)")
            continue
        print(f"Using msvc: {preferred_gen}")
        generator = preferred_gen
    else:
        generator = get_generator(preferred_gen)

    runs = [(std, opt, None, []) for std, opt in tests]
    runs += [
        (std, opt, tag, defines)
        for name, std, opt, tag, defines in extra_tests
        if name == compiler_name
    ]

    for std, opt, tag, defines in runs:

        if compiler_name == "msvc" and std == "c99":
            print("msvc c99 skipped (MSVC has no C99 mode)")
            continue

        opt_tag = opt.replace("-", "O")

        key = f"{compiler_name}-{std}-{opt}"
        if tag:
            key += f"-{tag}"
            opt_tag += f"-{tag}"

        build_dir = os.path.join(
            BUILD_ROOT,
            compiler_name,
            std,
            opt_tag
        )

        shutil.rmtree(build_dir, ignore_errors=True)

        # -----------------------------
        # Configure
        # -----------------------------
        cmake_cmd = [
            "cmake",
            "-S", BASE_DIR,
            "-B", build_dir,
            "-G", generator,
            f"-DCOMPILER_NAME={compiler_name}",
            f"-DC_STD={std}",
            f"-DOPT_LEVEL={opt}",
        ]

        if compiler:
            cmake_cmd.append(f"-DCMAKE_C_COMPILER={compiler}")

        if defines:
            flags = " ".join(f"-D{d}" for d in defines)
            cmake_cmd.append(f"-DCMAKE_C_FLAGS={flags}")

        if SANITIZERS:
            cmake_cmd.append(f"-DSANITIZERS={SANITIZERS}")

        build_cmd = ["cmake", "--build", build_dir]

        if generator.startswith("Visual Studio"):
            build_cmd += ["--config", "Release"]

        # -----------------------------
        # Configure + build (a failure is recorded, not fatal)
        # -----------------------------
        try:
            run(cmake_cmd)
            run(build_cmd)
        except subprocess.CalledProcessError:
            failures.append((key, "configure/build failed"))
            print(f"FAIL: {key} (configure/build failed)")
            continue

        # -----------------------------
        # Executable path
        # -----------------------------
        if generator.startswith("Visual Studio"):
            exe = os.path.join(build_dir, "Release", EXE_NAME)
        else:
            exe = os.path.join(build_dir, EXE_NAME)

        if not os.path.exists(exe):
            print(f"Missing executable: {exe}")
            failures.append((key, "missing executable"))
            continue

        # -----------------------------
        # Run
        # -----------------------------
        code = run_exe(exe)

        if code != 0:
            failures.append((key, f"exit code {code}"))
            print(f"FAIL: {key} (exit {code})")
        else:
            successes.append(key)
            print(f"PASS: {key}")


# -----------------------------
# Summary
# -----------------------------
print("\n=== TEST SUMMARY ===")

print("\nSuccessful runs:")
if successes:
    for s in successes:
        print(f"  {s}")
else:
    print("  None")

print("\nFailures:")
if failures:
    for name, reason in failures:
        print(f"  {name} -> {reason}")
else:
    print("  None")

print(f"\nTotal: {len(successes) + len(failures)}")
print(f"Passed: {len(successes)}")
print(f"Failed: {len(failures)}")

# Non-zero exit code so CI marks the run as failed
sys.exit(1 if failures else 0)