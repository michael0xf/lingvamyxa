"""Focused L1 import-storage regression; never promotes the shared compiler."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seed", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    seed = args.seed.resolve()
    seed_hash = sha(seed)
    output = (args.output or root / "build" / "import_capacity" /
              time.strftime("run_%Y%m%d_%H%M%S")).resolve()
    output.mkdir(parents=True, exist_ok=False)
    cc = shutil.which("gcc")
    if not cc:
        raise RuntimeError("gcc missing")
    flags = ["-std=c99", "-Wall", "-Wextra", "-Wpedantic",
             "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
             "-Werror=implicit-function-declaration", "-Werror=implicit-int",
             "-Werror=use-after-free",
             "-I", str(root), "-I", str(root / "lm1/build")]
    records = []

    def run(name, command, cwd=root, expect=0, diagnostic=None):
        command = [str(x) for x in command]
        result = subprocess.run(command, cwd=cwd, capture_output=True, timeout=90)
        (output / (name + ".stdout")).write_bytes(result.stdout)
        (output / (name + ".stderr")).write_bytes(result.stderr)
        records.append({"name": name, "command": command, "cwd": str(cwd),
                        "exit": result.returncode})
        (output / "commands.json").write_text(json.dumps(records, indent=2),
                                             encoding="utf-8")
        if (expect == 0 and result.returncode != 0) or (
                expect != 0 and result.returncode == 0):
            raise AssertionError(f"{name}: exit {result.returncode}; "
                                 f"{result.stderr.decode(errors='replace')[-2000:]}")
        if diagnostic and diagnostic not in result.stderr.decode(errors="replace"):
            raise AssertionError(f"{name}: missing {diagnostic!r}")
        print(f"{name}: PASS (exit {result.returncode})", flush=True)

    source = root / "l1src/l1trans.lm1"
    generated = output / "l1trans.c"
    candidate = output / "l1trans.exe"
    run("build_translate", [seed, source, generated])
    run("build_cc", [cc, *flags, "-o", candidate, generated])
    next_c = output / "l1trans_next.c"
    next_exe = output / "l1trans_next.exe"
    run("self_translate", [candidate, source, next_c])
    assert sha(next_c) == sha(generated), "self-generated C changed"
    run("self_cc", [cc, *flags, "-o", next_exe, next_c])
    final_c = output / "l1trans_check.c"
    run("self_check", [next_exe, source, final_c])
    assert sha(final_c) == sha(generated), "next-generation C changed"

    memory = output / "import_capacity_memory.exe"
    generated_define = '-DL1_IMPORT_GENERATED_C="' + generated.as_posix() + '"'
    run("memory_cc", [cc, *flags, generated_define, "-o", memory,
                      root / "tests/l1/import_capacity_memory.c"])
    run("memory", [memory])

    fixtures = output / "fixtures"
    fixtures.mkdir()

    def fixture(name, text):
        path = fixtures / name
        path.write_text(text, encoding="utf-8")
        return path

    def program(body="return: 0"):
        return "external:\n    fn: main () int\n        " + body + "\n"

    for i in range(65):
        fixture(f"mod{i}.lm1", f"fn: imported_{i} () int\nreturn: {i}\n")
    for count in (17, 65):
        text = "".join(f'predef: "mod{i}.lm1"\n' for i in range(count))
        text += 'predef: "mod0.lm1"\n'
        text += program(f"return: imported_0() + imported_{count-1}() - {count-1}")
        src = fixture(f"wide{count}.lm1", text)
        c = output / f"wide{count}.c"
        exe = output / f"wide{count}.exe"
        run(f"wide{count}_translate", [candidate, src, c])
        assert c.read_text().count("int imported_0(void)\n{") == 1
        run(f"wide{count}_cc", [cc, *flags, "-o", exe, c])
        run(f"wide{count}_run", [exe])

    for i in range(17):
        fixture(f"depth{i}.lm1", (f'predef: "depth{i+1}.lm1"\n'
                                if i < 16 else "fn: deepest () int\nreturn: 0\n"))
    depth_root = fixture("depth_root.lm1", 'predef: "depth1.lm1"\n' +
                         program("return: deepest()"))
    run("depth16", [candidate, depth_root, output / "depth16.c"])
    depth17 = fixture("depth17.lm1", 'predef: "depth0.lm1"\n' + program())
    run("depth17", [candidate, depth17, output / "depth17.c"])
    for i in range(65):
        fixture(f"deep{i}.lm1", (f'predef: "deep{i+1}.lm1"\n' if i < 64
                                else "fn: deep_leaf () int\nreturn: 0\n"))
        fixture(f"deep_cycle{i}.lm1", f'predef: "deep_cycle{(i+1) if i < 64 else 32}.lm1"\n')
    deep_root = fixture("deep_root.lm1", 'predef: "deep0.lm1"\n' +
                        program("return: deep_leaf()"))
    run("depth65", [candidate, deep_root, output / "depth65.c"])
    run("depth65_cc", [cc, *flags, "-o", output / "depth65.exe", output / "depth65.c"])
    run("depth65_run", [output / "depth65.exe"])
    fixture("cycle_a.lm1", 'predef: "cycle_b.lm1"\n')
    cycle = fixture("cycle_b.lm1", 'predef: "cycle_a.lm1"\n')
    missing = fixture("missing.lm1", 'predef: "missing_target.lm1"\n')
    for name, src, diagnostic in (
            ("deep_cycle", fixtures / "deep_cycle0.lm1", "import cycle"),
            ("cycle", cycle, "import cycle"),
            ("missing", missing, "cannot read import")):
        target = output / f"{name}.c"
        target.write_bytes(b"preserved-output\n")
        run(name, [candidate, src, target], expect=1, diagnostic=diagnostic)
        assert target.read_bytes() == b"preserved-output\n"
        assert not target.with_suffix(".c.tmp").exists()

    for i in range(20):
        fixture(f"hdr{i}.h.lm1", f"struct: ImportedType{i}\n    int: value\n")
    header_text = "".join(f'predef: "hdr{i}.h.lm1"\n' for i in range(20))
    header_root = fixture("headers.h.lm1", header_text +
                          'predef: "hdr0.h.lm1"\nstruct: UsesLast\n'
                          '    @: ImportedType19 last\n')
    run("header20", [candidate, "--unit-root", output, header_root,
                     output / "headers.lm1.h"])
    for i in range(33):
        fixture(f"deep_hdr{i}.h.lm1", (f'predef: "deep_hdr{i+1}.h.lm1"\n' if i < 32
                                       else "struct: DeepHeader\n    int: value\n"))
    run("header_depth33", [candidate, "--unit-root", output,
                           fixtures / "deep_hdr0.h.lm1", output / "deep_header.lm1.h"])
    header_cycle = fixture("hdr_cycle_a.h.lm1", 'predef: "hdr_cycle_b.h.lm1"\n')
    fixture("hdr_cycle_b.h.lm1", 'predef: "hdr_cycle_a.h.lm1"\n')
    run("header_cycle", [candidate, "--unit-root", output, header_cycle,
                         output / "header_cycle.lm1.h"], expect=1,
        diagnostic="import cycle")


    # Real import and output paths beyond the former 1040-byte compiler buffers.
    deep = fixtures
    while len(str(deep)) < 1200:
        deep /= "segment_" + "x" * 70
    deep.mkdir(parents=True)

    def native(path):
        # Extended Windows paths avoid the host's legacy MAX_PATH API limit.
        return chr(92) * 2 + "?" + chr(92) + str(path) if os.name == "nt" else str(path)

    (deep / "leaf.lm1").write_text("fn: long_leaf () int\nreturn: 19\n")
    (deep / "imported.lm1").write_text('predef: "leaf.lm1"\n')
    long_import = fixture("long_import.lm1", 'predef: "' +
                          native(deep / "imported.lm1") + '"\n' +
                          program("return: long_leaf() - 19"))
    long_c = output / "long_import.c"
    long_exe = output / "long_import.exe"
    run("long_import_translate", [candidate, long_import, long_c])
    run("long_import_cc", [cc, *flags, "-o", long_exe, long_c])
    run("long_import_run", [long_exe])

    (deep / "child.h.lm1").write_text("struct: LongChild\n    int: value\n")
    (deep / "unit.h.lm1").write_text('predef: "child.h.lm1"\n'
                                    'struct: LongUnit\n    @: LongChild child\n')
    (deep / "unit_use.lm1").write_text('predef: "unit.h.lm1"\n' + program())
    (deep / "missing.lm1").write_text('predef: "absent.lm1"\n')
    failed_target = deep / "preserved.c"
    failed_target.write_bytes(b"preserved-output\n")
    # Invoke generated main with explicit argv to bypass MinGW startup wildcard
    # expansion of the '?' in Windows extended paths. This tests the translator
    # including CLI handling; it does not change its standard CRT startup.
    cases = [(output, "child.h.lm1", "child.lm1.h", 0),
             (output, "unit.h.lm1", "unit.lm1.h", 0),
             (output, "unit_use.lm1", "unit_use.c", 0),
             (deep, "unit.h.lm1", "local_root.lm1.h", 0),
             (output, "missing.lm1", "preserved.c", 1)]
    driver = output / "long_paths_driver.c"
    code = '#define main translator_main\n#include ' + json.dumps(generated.as_posix())
    code += '\n#undef main\nint main(void) {\n'
    for index, (unit_root, src, dst, expected) in enumerate(cases):
        values = ["l1trans", "--unit-root", native(unit_root),
                  native(deep / src), native(deep / dst)]
        code += '  { char *args[] = {' + ','.join(json.dumps(v) for v in values) + '};\n'
        code += f'    if (translator_main(5, args) != {expected}) return {index+1}; }}\n'
    code += '  return 0;\n}\n'
    driver.write_text(code)
    driver_exe = output / "long_paths_driver.exe"
    run("long_paths_cc", [cc, *flags, "-o", driver_exe, driver])
    run("long_paths", [driver_exe])
    assert (deep / "child.lm1.h").exists()
    assert len(next(line for line in (deep / "unit.lm1.h").read_text().splitlines()
                    if line.startswith('#include "'))) > 1040
    assert 'child.lm1.h' in (deep / "local_root.lm1.h").read_text()
    assert '#include "' in (deep / "unit_use.c").read_text()
    assert failed_target.read_bytes() == b"preserved-output\n"
    assert not Path(str(failed_target) + ".tmp").exists()

    # Exact seven-predef reproducer supplied by Claude (mixa_audio.txt).
    imports = ["mixa_audio_panel.h.lm1", "mixa_audio_panel.lm1",
               "mixa_dir.h.lm1", "mixa_dir_win32.h.lm1", "mixa_dir_win32.lm1",
               "mixa_audio_launch.h.lm1", "mixa_audio_launch.lm1"]
    mp3 = fixture("mp3_reproducer.lm1", "".join(
        f'predef: "mixa_manager/{name}"\n' for name in imports) + program())
    run("mp3_old_reproduces", [seed, "--unit-root", root, mp3,
                              output / "mp3_old.c"], expect=1,
        diagnostic="import path table full")
    run("mp3_candidate", [candidate, "--unit-root", root, mp3,
                          output / "mp3.c"])

    # Compile the same targeted change in the baseline source mirror.
    baseline = root / "stg/l1_baseline"
    baseline_c = output / "baseline_l1trans.c"
    baseline_exe = output / "baseline_l1trans.exe"
    run("baseline_translate", [seed, "l1src/l1trans.lm1", baseline_c], cwd=baseline)
    run("baseline_cc", [cc, "-I", baseline, "-I", baseline / "lm1/build", *flags,
                        "-o", baseline_exe, baseline_c], cwd=baseline)
    run("baseline_wide65", [baseline_exe, fixtures / "wide65.lm1",
                           output / "baseline_wide65.c"], cwd=baseline)
    assert sha(seed) == seed_hash, "seed compiler changed"
    manifest = {"seed": str(seed), "seed_sha256": seed_hash,
                "candidate": str(candidate), "candidate_sha256": sha(candidate),
                "source_sha256": sha(source),
                "baseline_source_sha256": sha(baseline / "l1src/l1trans.lm1"),
                "generated_c_sha256": sha(generated),
                "long_paths_driver_sha256": sha(driver),
                "runner_sha256": sha(Path(__file__)),
                "memory_test_sha256": sha(root / "tests/l1/import_capacity_memory.c"),
                "headers_sha256": {str(p.relative_to(root)): sha(p) for p in
                    (root / "lm1/build/l1src/p0.lm1.h",
                     baseline / "lm1/build/l1src/p0.lm1.h")},
                "cc": cc, "cc_sha256": sha(Path(cc)),
                "checks": len(records),
                "limitations": [
                    "No stable/bootstrap publication.",
                    "No fixed import-count, import-depth or 1040-byte compiler path-buffer cap; native resources still bound recursion/allocation.",
                    "Windows extended-path CLI tests bypass MinGW wildcard startup by invoking generated main with explicit argv.",
                    "MP3 reproducer translated; full native app composition belongs to Claude."
                ]}
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2),
                                         encoding="utf-8")
    print(f"PASS: {len(records)} command checks; evidence {output}", flush=True)


if __name__ == "__main__":
    main()
