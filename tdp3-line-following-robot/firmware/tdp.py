#!/usr/bin/env python3
"""
tdp.py - build / flash / monitor tool for the TDP3 Team 7 FRDM-KL25Z firmware.

First time only (creates the venv and downloads the Arm compiler):
    python3 tdp.py setup

Every day:
    python tdp.py build                  compile -> build/tdp3.bin
    python tdp.py flash --build          compile, then copy onto the board
    python tdp.py flash                  copy the last build onto the board
    python tdp.py monitor                show the robot's debug messages
    python tdp.py clean                  delete the build folder
    python tdp.py doctor                 check what is installed / connected
    python tdp.py vscode                 (re)install the VSCode tasks + settings

build and flash take --mode demo|calibrate|motortest (default: config.h).
In VSCode you don't need any of this: Cmd+Shift+B builds, and the other
commands are under Terminal > Run Task.

Nothing here needs admin rights. Everything installed goes in ~/.tdp3
(outside OneDrive), so the project folder stays small and syncs cleanly.
"""

import argparse
import hashlib
import os
import platform
import re
import shutil
import subprocess
import sys
import time
import zipfile
from pathlib import Path

# ---------------------------------------------------------------- settings --
PROJECT = Path(__file__).resolve().parent
BUILD_DIR = PROJECT / "build"
NAME = "tdp3"

TDP_HOME = Path(os.environ.get("TDP3_HOME", str(Path.home() / ".tdp3")))
VENV_DIR = TDP_HOME / "venv"
TOOLCHAIN_DIR = TDP_HOME / "arm-gnu-toolchain"

TC_VERSION = "14.2.rel1"
TC_BASE_URL = "https://developer.arm.com/-/media/Files/downloads/gnu/{v}/binrel/".format(v=TC_VERSION)

IS_WINDOWS = os.name == "nt"
EXE = ".exe" if IS_WINDOWS else ""

MODES = {"demo": 0, "calibrate": 1, "motortest": 2}

SOURCE_DIRS = ["src", "platform"]
LINKER_SCRIPT = "platform/mkl25z128.ld"

CPU_FLAGS = ["-mcpu=cortex-m0plus", "-mthumb"]
C_FLAGS = CPU_FLAGS + [
    "-std=c11", "-g3",
    "-Wall", "-Wextra", "-Wshadow", "-Wdouble-promotion",
    "-ffunction-sections", "-fdata-sections", "-fno-common",
    "-Isrc", "-Iplatform",
]
LD_FLAGS = CPU_FLAGS + [
    "-T", LINKER_SCRIPT,
    "-nostartfiles", "--specs=nano.specs", "--specs=nosys.specs",
    "-Wl,--gc-sections", "-Wl,--print-memory-usage",
]

# Names / files that identify the FRDM board's USB drive.
BOARD_DRIVE_NAMES = {"MBED", "DAPLINK", "FRDM-KL25Z", "FRDMKL25Z", "KL25Z"}
BOARD_DRIVE_MARKERS = ("MBED.HTM", "DETAILS.TXT", "SDA_INFO.HTM", "LASTSTAT.TXT")
# USB vendor IDs of the FRDM's OpenSDA interface chip (Arm Mbed/DAPLink, P&E).
BOARD_USB_VIDS = {0x0D28, 0x1357}


# ------------------------------------------------------------------ helpers --
def say(msg=""):
    print(msg, flush=True)


def die(msg, code=1):
    say("")
    say("ERROR: " + msg)
    sys.exit(code)


def venv_python():
    if IS_WINDOWS:
        return VENV_DIR / "Scripts" / "python.exe"
    return VENV_DIR / "bin" / "python"


def running_in_our_venv():
    try:
        return Path(sys.prefix).resolve() == VENV_DIR.resolve()
    except OSError:
        return False


def toolchain_bin():
    """Folder holding arm-none-eabi-gcc: our pinned copy first, then PATH."""
    ours = TOOLCHAIN_DIR / "bin"
    if (ours / ("arm-none-eabi-gcc" + EXE)).exists():
        return ours
    found = shutil.which("arm-none-eabi-gcc")
    if found:
        return Path(found).resolve().parent
    return None


def tool(name):
    bindir = toolchain_bin()
    if bindir is None:
        die("The Arm compiler isn't installed yet. Run:  python3 tdp.py setup")
    return str(bindir / ("arm-none-eabi-" + name + EXE))


def host_id():
    system = platform.system()
    machine = platform.machine().lower()
    if system == "Darwin":
        # A Python running under Rosetta reports x86_64 even on Apple silicon,
        # so ask the hardware directly.
        try:
            out = subprocess.run(["sysctl", "-n", "hw.optional.arm64"],
                                 capture_output=True, text=True).stdout.strip()
            if out == "1":
                return "darwin-arm64"
        except OSError:
            pass
        return "darwin-arm64" if machine in ("arm64", "aarch64") else "darwin-x86_64"
    if system == "Linux":
        return "aarch64" if machine in ("aarch64", "arm64") else "x86_64"
    if system == "Windows":
        return "mingw-w64-x86_64" if platform.machine().endswith("64") else "mingw-w64-i686"
    die("Unsupported operating system: " + system)


def download(url, dest):
    dest = Path(dest)
    tmp = dest.with_name(dest.name + ".part")
    curl = shutil.which("curl")
    if curl:
        rc = subprocess.run([curl, "-L", "--fail", "--retry", "3", "-o", str(tmp), url]).returncode
        if rc != 0:
            if tmp.exists():
                tmp.unlink()
            raise RuntimeError("download failed ({}): {}".format(rc, url))
    else:
        import urllib.request
        with urllib.request.urlopen(url) as resp, open(str(tmp), "wb") as out:
            shutil.copyfileobj(resp, out)
    tmp.replace(dest)


def sha256_of(path):
    h = hashlib.sha256()
    with open(str(path), "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


# -------------------------------------------------------------------- setup --
def install_toolchain():
    host = host_id()
    ext = ".zip" if IS_WINDOWS else ".tar.xz"
    fname = "arm-gnu-toolchain-{}-{}-arm-none-eabi{}".format(TC_VERSION, host, ext)
    url = TC_BASE_URL + fname
    dl_dir = TDP_HOME / "downloads"
    dl_dir.mkdir(parents=True, exist_ok=True)
    archive = dl_dir / fname

    say("Downloading Arm GNU Toolchain {} for {} (about 150 MB, one time only)".format(TC_VERSION, host))
    say("  " + url)
    if not archive.exists():
        download(url, archive)

    # Verify against Arm's published checksum when it is available.
    try:
        sum_file = dl_dir / (fname + ".sha256asc")
        download(url + ".sha256asc", sum_file)
        expected = sum_file.read_text().split()[0].lower()
        actual = sha256_of(archive)
        if expected != actual:
            archive.unlink()
            die("Checksum mismatch on the downloaded compiler (deleted it). Run setup again.")
        say("  checksum OK")
    except Exception:
        say("  (couldn't fetch Arm's checksum file; skipping the check)")

    say("Unpacking...")
    staging = TDP_HOME / "unpack-tmp"
    if staging.exists():
        shutil.rmtree(str(staging))
    staging.mkdir(parents=True)
    if IS_WINDOWS:
        with zipfile.ZipFile(str(archive)) as z:
            z.extractall(str(staging))
    else:
        subprocess.run(["tar", "-xf", str(archive), "-C", str(staging)], check=True)

    gcc_name = "arm-none-eabi-gcc" + EXE
    root = None
    for cand in [staging] + [p for p in staging.iterdir() if p.is_dir()]:
        if (cand / "bin" / gcc_name).exists():
            root = cand
            break
    if root is None:
        die("Couldn't find arm-none-eabi-gcc inside the downloaded archive.")

    if TOOLCHAIN_DIR.exists():
        shutil.rmtree(str(TOOLCHAIN_DIR))
    shutil.move(str(root), str(TOOLCHAIN_DIR))
    (TOOLCHAIN_DIR / "TDP3_VERSION.txt").write_text(TC_VERSION + " " + host + "\n")
    shutil.rmtree(str(staging), ignore_errors=True)
    archive.unlink()   # free ~150 MB; the unpacked copy is all we need
    say("Compiler installed in " + str(TOOLCHAIN_DIR))


def cmd_setup(args):
    say("=== TDP3 setup ===")
    if sys.version_info < (3, 8):
        die("Python 3.8 or newer is needed (you have {}).".format(platform.python_version()))
    TDP_HOME.mkdir(parents=True, exist_ok=True)

    # 1. Python virtual environment
    if not venv_python().exists():
        say("Creating virtual environment: " + str(VENV_DIR))
        subprocess.run([sys.executable, "-m", "venv", str(VENV_DIR)], check=True)
    else:
        say("Virtual environment already exists: " + str(VENV_DIR))
    say("Installing Python packages (requirements.txt)...")
    subprocess.run([str(venv_python()), "-m", "pip", "install", "--disable-pip-version-check",
                    "-q", "-r", str(PROJECT / "requirements.txt")], check=True)

    # 2. Arm cross-compiler
    if args.system_gcc:
        if not shutil.which("arm-none-eabi-gcc"):
            die("--system-gcc was given but arm-none-eabi-gcc is not on PATH.")
        say("Using the arm-none-eabi-gcc already on PATH.")
    elif (TOOLCHAIN_DIR / "bin" / ("arm-none-eabi-gcc" + EXE)).exists() and not args.force:
        say("Arm compiler already installed: " + str(TOOLCHAIN_DIR))
    else:
        install_toolchain()

    out = subprocess.run([tool("gcc"), "--version"], capture_output=True, text=True).stdout
    say("Compiler: " + out.splitlines()[0])

    # 3. VSCode tasks / settings
    if not args.no_vscode:
        say("")
        say("VSCode config:")
        cmd_vscode(args)

    # 4. Prove it works
    say("")
    say("Test build:")
    rc = cmd_build(argparse.Namespace(mode=None, debug=False, quiet=True))
    if rc != 0:
        die("Setup finished but the test build failed (see above).")

    say("")
    say("=== All set ===")
    say("Venv:      " + str(VENV_DIR))
    say("Compiler:  " + str(toolchain_bin()))
    say("In VSCode: open this folder, then Cmd+Shift+B to build,")
    say("           Terminal > Run Task > 'TDP3: Build + Flash' to program the board.")
    return 0


# -------------------------------------------------------------------- build --
def cmd_build(args):
    gcc, objcopy, size = tool("gcc"), tool("objcopy"), tool("size")

    obj_dir = BUILD_DIR / "obj"
    if obj_dir.exists():
        shutil.rmtree(str(obj_dir))   # always a clean build: it takes ~1 s
    obj_dir.mkdir(parents=True)

    defines = []
    mode_text = "from config.h"
    if getattr(args, "mode", None):
        defines.append("-DRUN_MODE={}".format(MODES[args.mode]))
        mode_text = args.mode
    opt = ["-Og"] if getattr(args, "debug", False) else ["-O2"]

    sources = []
    for d in SOURCE_DIRS:
        sources += sorted((PROJECT / d).glob("*.c"))

    say("Building ({} mode, {})".format(mode_text, opt[0]))
    objects = []
    for src in sources:
        rel = src.relative_to(PROJECT).as_posix()
        obj = obj_dir / (rel.replace("/", "_") + ".o")
        say("  CC  " + rel)
        rc = subprocess.run([gcc] + C_FLAGS + opt + defines + ["-c", rel, "-o", str(obj)],
                            cwd=str(PROJECT)).returncode
        if rc != 0:
            say("BUILD FAILED")
            return rc
        objects.append(str(obj))

    elf = BUILD_DIR / (NAME + ".elf")
    say("  LD  " + elf.relative_to(PROJECT).as_posix())
    rc = subprocess.run([gcc] + objects + LD_FLAGS +
                        ["-Wl,-Map=" + str(BUILD_DIR / (NAME + ".map")), "-o", str(elf)],
                        cwd=str(PROJECT)).returncode
    if rc != 0:
        say("LINK FAILED")
        return rc

    for fmt, ext in (("binary", ".bin"), ("ihex", ".hex"), ("srec", ".srec")):
        subprocess.run([objcopy, "-O", fmt, str(elf), str(BUILD_DIR / (NAME + ext))], check=True)
    subprocess.run([size, str(elf)], cwd=str(PROJECT))

    binf = BUILD_DIR / (NAME + ".bin")
    (BUILD_DIR / "build-info.txt").write_text(
        "mode: {}\noptimisation: {}\nbuilt: {}\n".format(mode_text, opt[0], time.strftime("%Y-%m-%d %H:%M:%S")))
    say("OK -> {} ({} bytes)".format(binf.relative_to(PROJECT).as_posix(), binf.stat().st_size))
    return 0


# -------------------------------------------------------------------- flash --
def candidate_mounts():
    roots = []
    system = platform.system()
    if system == "Darwin":
        base = Path("/Volumes")
        if base.exists():
            roots = [p for p in base.iterdir()]
    elif system == "Linux":
        # /media/<user>/<LABEL>, /run/media/<user>/<LABEL>, /media/<LABEL>, /mnt/<LABEL>
        for base in (Path("/media"), Path("/run/media"), Path("/mnt")):
            if not base.exists():
                continue
            for p in base.iterdir():
                roots.append(p)
                if p.is_dir() and base.name in ("media",):
                    try:
                        roots += [q for q in p.iterdir()]
                    except OSError:
                        pass
    elif IS_WINDOWS:
        for letter in "CDEFGHIJKLMNOPQRSTUVWXYZ":
            p = Path(letter + ":/")
            if os.path.exists(str(p)):
                roots.append(p)
    return roots


def find_board_drive():
    """Return (drive, bootloader_drive, blocked_volumes). drive/bootloader may be None."""
    found, bootloader, blocked = [], None, []
    for root in candidate_mounts():
        try:
            if not root.is_dir():
                continue
            name = root.name.upper()
            if name in ("BOOTLOADER", "MAINTENANCE"):
                bootloader = root
                continue
            if name in BOARD_DRIVE_NAMES or "KL25" in name:
                found.append(root)
                continue
            if any((root / m).exists() for m in BOARD_DRIVE_MARKERS):
                found.append(root)
        except PermissionError:
            blocked.append(root)
        except OSError:
            continue
    if len(found) > 1:
        say("More than one board drive found: " + ", ".join(str(f) for f in found))
        say("Using the first. Pick one with --drive if that's wrong.")
    return (found[0] if found else None), bootloader, blocked


def usb_device_names():
    """Names of the USB devices the computer can see (macOS: ioreg, Linux: lsusb). Best effort."""
    names = []
    try:
        if platform.system() == "Darwin":
            out = subprocess.run(["ioreg", "-p", "IOUSB", "-w0"], capture_output=True,
                                 text=True, timeout=10).stdout
            for m in re.finditer(r"\+-o (.+?)@[0-9A-Fa-f]+\s+<class (?:IOUSBHostDevice|IOUSBDevice)\b", out):
                names.append(m.group(1).strip())
        elif platform.system() == "Linux" and shutil.which("lsusb"):
            out = subprocess.run(["lsusb"], capture_output=True, text=True, timeout=10).stdout
            for line in out.splitlines():
                m = re.search(r"ID ([0-9a-fA-F]{4}:[0-9a-fA-F]{4})\s*(.*)", line)
                if m and "root hub" not in m.group(2).lower():
                    names.append("{} ({})".format(m.group(2).strip() or "unknown", m.group(1).lower()))
    except (OSError, subprocess.SubprocessError):
        pass
    return names


def board_interface_kind(usb_names):
    """Guess which firmware the FRDM's OpenSDA chip runs, from USB device names."""
    text = " | ".join(usb_names).lower()
    if "j-link" in text or "segger" in text:
        return "jlink"
    if "cmsis-dap" in text or "daplink" in text or "mbed" in text or "0d28:" in text:
        return "cmsis-dap"
    if "opensda" in text or "pemicro" in text or "p&e" in text or "1357:" in text:
        return "pe"
    return None


def explain_missing_drive(blocked):
    """Print what the computer can see, and the most likely reason the board's drive is missing."""
    volumes = [p.name for p in candidate_mounts()]
    usb = usb_device_names()
    say("")
    say("What this computer can see right now:")
    say("  Drives:      " + (", ".join(volumes) if volumes else "(none)"))
    if platform.system() in ("Darwin", "Linux"):
        say("  USB devices: " + (", ".join(usb) if usb else "(nothing plugged in)"))
    say("")
    if blocked:
        say("This app isn't allowed to look inside: " + ", ".join(str(b) for b in blocked))
        say("  macOS: System Settings > Privacy & Security > Files and Folders >")
        say("  Visual Studio Code (or Terminal) > turn on 'Removable Volumes', then try again.")
        return
    kind = board_interface_kind(usb)
    if kind == "jlink":
        say("The board is connected, but its USB chip runs SEGGER J-Link firmware, which has")
        say("no drag-and-drop drive. It needs the Mbed/DAPLink firmware loaded instead.")
    elif kind == "cmsis-dap":
        say("The board's USB chip is connected, but its drive isn't mounted.")
        say("Unplug it, wait 5 seconds and plug it back in, then try again.")
    elif kind == "pe":
        say("The board is connected with P&E OpenSDA firmware, but no drive is mounted.")
        say("It may be a debug-only version without drag-and-drop.")
    else:
        say("The computer can't see the board at all. Most likely, in order:")
        say("  1. Wrong USB port: use the one labelled SDA on the board, not KL25Z.")
        say("  2. Charge-only cable: try a cable you know transfers data.")
        if platform.system() == "Darwin":
            say("  3. macOS asked 'Allow accessory to connect?' and it was missed:")
            say("     unplug, plug back in, click Allow.")
            say("  4. USB-C hub or adapter: try another, or plug straight into the Mac.")


def wait_for_remount(drive, timeout=20.0):
    """The board unmounts its drive while programming, then mounts it again."""
    t0 = time.time()
    gone = False
    while time.time() - t0 < timeout:
        if not drive.exists():
            gone = True
        elif gone:
            time.sleep(1.0)
            return True
        time.sleep(0.25)
    return False


def cmd_flash(args):
    if args.build:
        rc = cmd_build(args)
        if rc != 0:
            return rc
    binf = BUILD_DIR / (NAME + ".bin")
    if not binf.exists():
        die("No build/tdp3.bin yet. Run:  python tdp.py flash --build")

    if args.drive:
        drive = Path(args.drive)
        bootloader, blocked = None, []
    else:
        drive, bootloader, blocked = find_board_drive()
    if drive is None:
        if bootloader is not None:
            die("The board is in BOOTLOADER mode (RESET was held while plugging in). "
                "Unplug it and plug it back in WITHOUT holding RESET.")
        say("")
        say("ERROR: Couldn't find the board's USB drive (MBED / DAPLINK / FRDM-KL25Z).")
        explain_missing_drive(blocked)
        say("")
        say("If you can see the drive in Finder, point at it:  python tdp.py flash --drive /Volumes/<NAME>")
        return 1

    # Mbed / DAPLink interface firmware takes .bin. The older P&E OpenSDA
    # firmware (drive contains SDA_INFO.HTM) is happiest with S-records.
    image = binf
    pe_firmware = (drive / "SDA_INFO.HTM").exists() and not (drive / "MBED.HTM").exists() \
        and not (drive / "DETAILS.TXT").exists()
    if pe_firmware:
        image = BUILD_DIR / (NAME + ".srec")
        say("Board has P&E OpenSDA firmware: sending the S-record file.")
    data = image.read_bytes()
    say("Copying {} ({} bytes) to {} ...".format(image.name, len(data), drive))
    dest = drive / image.name
    try:
        # Plain data copy: no macOS extended attributes / ._ files, which can
        # confuse the board's programmer.
        with open(str(dest), "wb") as f:
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
    except PermissionError:
        die("Not allowed to write to {}.\n"
            "  macOS: System Settings > Privacy & Security > Files and Folders >\n"
            "  Visual Studio Code (or Terminal) > turn on 'Removable Volumes'.".format(drive))
    except OSError as e:
        say("  (the drive went away during the copy: {} - usually this just means".format(e))
        say("   the board started programming straight away)")
    if hasattr(os, "sync"):
        os.sync()

    say("Waiting for the board to program itself...")
    remounted = wait_for_remount(drive)
    fail = drive / "FAIL.TXT"
    if remounted and fail.exists():
        try:
            reason = fail.read_text(errors="replace").strip()
        except OSError:
            reason = "(couldn't read FAIL.TXT)"
        die("The board rejected the file: " + reason)
    laststat = drive / "LASTSTAT.TXT"   # P&E firmware status file
    if remounted and pe_firmware and laststat.exists():
        try:
            say("  board status: " + laststat.read_text(errors="replace").strip())
        except OSError:
            pass
    if not remounted:
        say("  (the drive didn't remount; check the board if the robot doesn't behave as expected)")
    say("Flashed. If the program doesn't start by itself, press RESET on the board.")
    return 0


# ------------------------------------------------------------------ monitor --
def find_serial_port(list_ports):
    best = None
    for p in list_ports.comports():
        text = " ".join(str(x) for x in (p.description, p.manufacturer, p.product) if x).lower()
        score = 0
        if p.vid in BOARD_USB_VIDS:
            score += 10
        if any(k in text for k in ("mbed", "daplink", "opensda", "kl25", "cmsis-dap")):
            score += 5
        if "usbmodem" in p.device:
            score += 2
        if score and (best is None or score > best[0]):
            best = (score, p.device)
    return best[1] if best else None


def cmd_monitor(args):
    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        if venv_python().exists() and not running_in_our_venv():
            return subprocess.call([str(venv_python()), str(Path(__file__).resolve())] + sys.argv[1:])
        die("pyserial isn't installed. Run:  python3 tdp.py setup")

    port = args.port or find_serial_port(list_ports)
    if port is None:
        say("Serial ports seen:")
        for p in list_ports.comports():
            say("  {}  {}".format(p.device, p.description))
        die("Couldn't find the board's serial port. Is the USB cable in the SDA port?")

    say("--- {} @ {} baud. Ctrl+C to quit. Press RESET on the board to see start-up. ---".format(port, args.baud))
    ser = None
    try:
        while True:
            if ser is None:
                try:
                    ser = serial.Serial(port, args.baud, timeout=0.2)
                except (serial.SerialException, OSError):
                    time.sleep(0.5)
                    continue
            try:
                data = ser.read(ser.in_waiting or 1)
            except (serial.SerialException, OSError):
                say("\n--- board disconnected (re-flashing?), waiting for it to come back ---")
                try:
                    ser.close()
                except Exception:
                    pass
                ser = None
                time.sleep(1.0)
                continue
            if data:
                sys.stdout.write(data.decode("utf-8", errors="replace").replace("\r\n", "\n"))
                sys.stdout.flush()
    except KeyboardInterrupt:
        say("\n--- monitor closed ---")
    finally:
        if ser is not None:
            ser.close()
    return 0


# ------------------------------------------------------------------- vscode --
def cmd_vscode(args):
    """Copy vscode-template/*.json into .vscode/ (tasks, settings, IntelliSense)."""
    src_dir = PROJECT / "vscode-template"
    dst_dir = PROJECT / ".vscode"
    dst_dir.mkdir(exist_ok=True)
    for src in sorted(src_dir.glob("*.json")):
        dst = dst_dir / src.name
        new = src.read_text()
        if dst.exists():
            old = dst.read_text()
            if old == new:
                say("  .vscode/{} is up to date".format(src.name))
                continue
            backup = dst.with_name(dst.name + ".bak")
            backup.write_text(old)
            say("  .vscode/{} replaced (old copy saved as {})".format(src.name, backup.name))
        else:
            say("  .vscode/{} installed".format(src.name))
        dst.write_text(new)
    say("VSCode config installed. Reload the window: Cmd+Shift+P -> 'Developer: Reload Window'.")
    return 0


# -------------------------------------------------------------- clean/doctor --
def cmd_clean(args):
    if BUILD_DIR.exists():
        shutil.rmtree(str(BUILD_DIR))
    say("Removed build/")
    return 0


def cmd_doctor(args):
    ok = True
    say("Python:     {} ({})".format(platform.python_version(), sys.executable))
    say("Venv:       {} {}".format(VENV_DIR, "OK" if venv_python().exists() else "MISSING - run setup"))
    ok &= venv_python().exists()
    bindir = toolchain_bin()
    if bindir:
        out = subprocess.run([str(bindir / ("arm-none-eabi-gcc" + EXE)), "--version"],
                             capture_output=True, text=True).stdout.splitlines()
        say("Compiler:   {} ({})".format(out[0] if out else "?", bindir))
    else:
        say("Compiler:   MISSING - run setup")
        ok = False
    say("VSCode cfg: {}".format("OK" if (PROJECT / ".vscode" / "tasks.json").exists() else "missing - run  python3 tdp.py vscode"))
    drive, boot, blocked = find_board_drive()
    say("Board USB:  {}".format(drive or ("BOOTLOADER mode - replug without holding RESET" if boot else "not found")))
    if drive is None and boot is None:
        explain_missing_drive(blocked)
        say("")
    try:
        from serial.tools import list_ports
        say("Serial:     {}".format(find_serial_port(list_ports) or "not found"))
    except ImportError:
        say("Serial:     (pyserial not available in this Python - run inside the venv)")
    say("Overall:    {}".format("ready" if ok else "run  python3 tdp.py setup"))
    return 0 if ok else 1


# --------------------------------------------------------------------- main --
def main():
    parser = argparse.ArgumentParser(description="TDP3 Team 7 FRDM-KL25Z tool",
                                     formatter_class=argparse.RawDescriptionHelpFormatter,
                                     epilog=__doc__)
    sub = parser.add_subparsers(dest="command")

    p = sub.add_parser("setup", help="create the venv and install the Arm compiler")
    p.add_argument("--system-gcc", action="store_true", help="use arm-none-eabi-gcc from PATH instead of downloading")
    p.add_argument("--force", action="store_true", help="re-download the compiler")
    p.add_argument("--no-vscode", action="store_true", help="don't install the .vscode/ config")
    p.set_defaults(func=cmd_setup)

    sub.add_parser("vscode", help="install the VSCode tasks/settings into .vscode/").set_defaults(func=cmd_vscode)

    for name, func, hlp in (("build", cmd_build, "compile the firmware"),
                            ("flash", cmd_flash, "copy build/tdp3.bin onto the board")):
        p = sub.add_parser(name, help=hlp)
        p.add_argument("--mode", choices=sorted(MODES), help="override RUN_MODE from config.h")
        p.add_argument("--debug", action="store_true", help="build with -Og for easier debugging")
        if name == "flash":
            p.add_argument("--build", action="store_true", help="build first")
            p.add_argument("--drive", help="board drive path, e.g. /Volumes/MBED")
        p.set_defaults(func=func)

    p = sub.add_parser("monitor", help="show debug messages from the board")
    p.add_argument("--port", help="serial port, e.g. /dev/cu.usbmodem1102")
    p.add_argument("--baud", type=int, default=115200)
    p.set_defaults(func=cmd_monitor)

    sub.add_parser("clean", help="delete build/").set_defaults(func=cmd_clean)
    sub.add_parser("doctor", help="check the setup").set_defaults(func=cmd_doctor)

    args = parser.parse_args()
    if not getattr(args, "func", None):
        parser.print_help()
        return 1
    return args.func(args) or 0


if __name__ == "__main__":
    sys.exit(main())
