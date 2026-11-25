"""Create the Edax runtime ZIP with the upstream 4.5.5 directory layout.

Usage: python package-release.py path/to/edax-4.5.5-heath.N-64.zip --slots 64|48 [--eval eval.dat] [--without-macos]
--slots: the feature slots of the binaries (EVAL_SLOTS of their build).
--eval: the eval.dat put in the ZIP as bin/data/eval.dat (default: bin/data/eval.dat of the repository, the
one of eval2, refused). It must hold the weights of the model of --slots, checked as Edax reads them
(eval_model below).
--without-macos makes a ZIP to check before the macOS files (built by the
release-binaries workflow) are there: it leaves them out.
The 32-bit macOS executable is intentionally omitted: current Xcode SDKs
cannot link it, and shipping the upstream executable would misrepresent it
as containing this fork's fixes.
"""

from hashlib import sha256
from os import replace
from pathlib import Path
from struct import unpack_from
from sys import argv
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo


ROOT = Path(__file__).resolve().parent

# an eval.dat of form 2 ("NEDXEVAL", docs/port/DESIGN-format.md): the header (size at 10, flags at 12, plies at 36,
# first ply at 38, weights of a ply at 40, groups at 44, the groups of 20 bytes from 96, CRC-32C), the weights, the trailer
EVAL2_GROUPS_AT = 96
EVAL2_GROUP_SIZE = 20
EVAL2_MOBILITY_GROUP = bytes((0, 2, 1, 0, 64)) + bytes(15)
# the extra groups of the 64-slot evaluation that the 48 slots leave out (src/eval/eval.cpp, EVAL_X_LEFT_OUT: X5 and
# X11), as groups of the file (after the 13 groups of Edax 4.5), with their squares
EVAL_LEFT_OUT = ((13 + 5, bytes((38, 45, 52, 53, 54, 59, 60, 61, 62, 63))),
                 (13 + 11, bytes((2, 3, 6, 7, 9, 11, 13, 14, 15, 21))))


def crc32c(data: bytes) -> int:
    """CRC-32C (Castagnoli), as crc32c_bytes of src/util"""
    crc = 0xFFFFFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0x82F63B78 if crc & 1 else crc >> 1
    return crc ^ 0xFFFFFFFF


def eval_model(data: bytes, name: str) -> str:
    """The model of the weights of an eval.dat, as src/eval/eval.cpp (eval_load) tells them apart: "old" (form 1, or
    form 2 without the extra groups: the weights of Edax 4.5 or of eval2, refused by Heath), "48" (the weights are 0
    in the groups the 48 slots leave out: read by the 48 and the 64 slots), "64" (refused by the 48 slots). A file that
    Edax does not read stops the script."""
    if data[:8] in (b"XADELAVE", b"EDAXEVAL"):
        return "old"
    if data[:8] != b"NEDXEVAL" or len(data) < 12:
        raise SystemExit(f"{name} is not an evaluation file")
    form, size = unpack_from("<HH", data, 8)
    if form != 2 or size < EVAL2_GROUPS_AT + 4 or len(data) < size:
        raise SystemExit(f"{name} is an evaluation file of another form")
    if crc32c(data[:size - 4]) != unpack_from("<I", data, size - 4)[0]:
        raise SystemExit(f"{name} has a damaged header")
    flags, = unpack_from("<I", data, 12)
    n_plies, first_ply, n_weights, n_group = unpack_from("<HHIH", data, 36)
    if flags != 0 or size != EVAL2_GROUPS_AT + n_group * EVAL2_GROUP_SIZE + 4 or first_ply != 0 or n_plies < 54:
        raise SystemExit(f"{name} holds weights that Edax does not read")
    groups = [data[EVAL2_GROUPS_AT + g * EVAL2_GROUP_SIZE:EVAL2_GROUPS_AT + (g + 1) * EVAL2_GROUP_SIZE] for g in range(n_group)]
    sizes = [unpack_from("<I", g, 4)[0] for g in groups]
    mobility = n_group > 1 and groups[-1] == EVAL2_MOBILITY_GROUP
    n_pattern = n_group - (1 if mobility else 0)
    if sum(sizes) != n_weights or len(data) != size + n_plies * n_weights * 2 + 8 or data[-4:] != b"EEND":
        raise SystemExit(f"{name} is damaged (the sizes of its groups, of its weights)")
    if n_pattern == 13:
        return "old"
    if n_pattern != 26 or any(groups[g][8:18] != squares for g, squares in EVAL_LEFT_OUT):
        raise SystemExit(f"{name} holds weights of patterns that Edax does not read")
    for ply in range(54):  # (the plies Edax reads)
        start = size + ply * n_weights * 2
        for g, _ in EVAL_LEFT_OUT:
            at = start + sum(sizes[:g]) * 2
            if data[at:at + sizes[g] * 2].strip(b"\0"):
                return "64"
    return "48"

BINARIES = (
    "bin/aEdax-arm64-v8a",
    "bin/aEdax-armeabi-v7a",
    "bin/lEdax-x86",
    "bin/lEdax-x86-64",
    "bin/lEdax-x86-64-v3",
    "bin/lEdax-x86-64-v4",
    "bin/mEdax-arm64",
    "bin/mEdax-x64-modern",
    "bin/wEdax-arm64.exe",
    "bin/wEdax-x86.exe",
    "bin/wEdax-x86-sse.exe",
    "bin/wEdax-x86-64.exe",
    "bin/wEdax-x86-64-v3.exe",
    "bin/wEdax-x86-64-v4.exe",
)

# libedax (Edax as a library)
LIBRARIES = (
    "bin/libedax.universal.dylib",
    "bin/libedax-arm64-v8a.so",
    "bin/libedax-armeabi-v7a.so",
    "bin/libedax-x86-64.so",
    "bin/libedax-x86-64-v3.so",
    "bin/libedax-x86-64-v4.so",
    "bin/libedax-x64.dll",
    "bin/libedax-x64-v3.dll",
    "bin/libedax-x64-v4.dll",
)

MACOS_FILES = ("bin/mEdax-arm64", "bin/mEdax-x64-modern", "bin/libedax.universal.dylib")

OTHER_FILES = (
    "LICENSE",
    "README.md",
    "RELEASE-NOTES.md",
    "RELEASE-NOTES.ja.md",
    "bin/README.MS-Windows.txt",
    "bin/config.ini",
    "bin/data/book.dat",
    "bin/data/eval.dat",
    "problem/README.md",
    "problem/fforum-1-19.obf",
    "problem/fforum-20-39.obf",
    "problem/fforum-40-59.obf",
    "problem/fforum-60-79.obf",
)


# the user manual (since v4.5.5-nikque.13): every page of manual/, manual/ja and manual/en
MANUAL_FILES = tuple(sorted(p.relative_to(ROOT).as_posix() for p in (ROOT / "manual").rglob("*.md")))


def main() -> None:
    if len(argv) < 2 or argv[1].startswith("--"):
        raise SystemExit(__doc__)
    output = Path(argv[1]).resolve()
    slots, eval_file, without_macos = None, ROOT / "bin/data/eval.dat", False
    rest = argv[2:]
    while rest:
        option = rest.pop(0)
        if option == "--without-macos":
            without_macos = True
        elif option in ("--slots", "--eval") and rest:
            value = rest.pop(0)
            if option == "--slots":
                slots = value
            else:
                eval_file = Path(value).resolve()
        else:
            raise SystemExit(f"unknown option, or option without its value: {option}\n\n{__doc__}")
    if slots not in ("64", "48"):
        raise SystemExit(f"--slots 64 or --slots 48 is needed\n\n{__doc__}")

    if not eval_file.is_file():
        raise SystemExit(f"missing file: {eval_file}")
    eval_data = eval_file.read_bytes()
    model = eval_model(eval_data, str(eval_file))
    if model != slots:
        held = "the weights of Edax 4.5 or of eval2" if model == "old" else f"the weights of the {model}-slot model"
        raise SystemExit(f"{eval_file} holds {held}, not those of the {slots} slots of the binaries")
    print(f"bin/data/eval.dat: {eval_file}, weights of the {model}-slot model, SHA-256 {sha256(eval_data).hexdigest()}")

    names = [name for name in (*OTHER_FILES, *MANUAL_FILES, *BINARIES, *LIBRARIES)
             if not (without_macos and name in MACOS_FILES)]
    for name in (*OTHER_FILES, *MANUAL_FILES, *BINARIES, *LIBRARIES):
        if name not in names:
            print(f"left out: {name}")
    # every file is checked before the ZIP is written, and the ZIP is written to another file first: a missing
    # file or a failure leaves no partial ZIP, nor replaces an existing one (up to v4.5.5-nikque.15 it did)
    missing = [name for name in names if name != "bin/data/eval.dat" and not (ROOT / name).is_file()]
    if missing:
        raise SystemExit("missing files: " + ", ".join(missing))

    output.parent.mkdir(parents=True, exist_ok=True)
    partial = output.with_name(output.name + ".tmp")
    try:
        with ZipFile(partial, "w") as archive:
            for name in names:
                data = eval_data if name == "bin/data/eval.dat" else (ROOT / name).read_bytes()
                info = ZipInfo(name, date_time=(2026, 9, 27, 0, 0, 0))
                info.create_system = 3
                executable = name in (*BINARIES, *LIBRARIES) and not name.endswith((".exe", ".dll"))
                info.external_attr = ((0o100755 if executable else 0o100644) << 16)
                info.compress_type = ZIP_DEFLATED
                archive.writestr(info, data, compress_type=ZIP_DEFLATED, compresslevel=9)

        with ZipFile(partial) as archive:
            bad = archive.testzip()
            if bad is not None:
                raise SystemExit(f"ZIP verification failed: {bad}")
        replace(partial, output)
    finally:
        if partial.exists():
            partial.unlink()
    print(f"Created {output} ({output.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
