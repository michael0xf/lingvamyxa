# Convert lm2/l1trans.lm2 (L2 C-like) into l1src/l1trans.lm2 (L1).
from __future__ import annotations

import importlib.util
from pathlib import Path

ROOT = Path(r"C:\Nyasha_Planet\lingvamyxa")
spec = importlib.util.spec_from_file_location("port_parser", ROOT / "l1src" / "port_parser.py")
pp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pp)

SRC = ROOT / "lm2" / "l1trans.lm2"
DST = ROOT / "l1src" / "l1trans.lm2"

# Seed lm2/l1trans.lm2 is compiled by trans.lm0 (L2). Do not put an L1
# prototype: frame there. Inject typed forward decls here, matching
# l1src/parser.lm2 HAND_PROTOS. Names must match later fn: definitions.
EMIT_PROTOS = """
    prototype:
        fn: l1_emit_atom_c (@: FILE out; const: @(LmP0Text t)) int
        fn: l1_emit_atom_node (@: FILE out; const: @(LmP0Node node)) int
        fn: l1_is_unary_prefix_atom (const: @(LmP0Text text)) int
        fn: l1_is_infix_atom (const: @(LmP0Text text)) int
        fn: l1_is_incdec_atom (const: @(LmP0Text text)) int
        fn: l1_emit_call_frame (@: FILE out; const: @(LmP0Frame frame); const: @(char path); const: @(LmP0Node node)) int
        fn: l1_emit_cast (@: FILE out; const: @(LmP0Structure body); const: @(char path)) int
        fn: l1_emit_arg_list (@: FILE out; const: @(LmP0Structure body); const: @(char path)) int
        fn: l1_emit_type_token (@: FILE out; const: @(LmP0Node node); const: @(char path)) int
        fn: l1_emit_item (@: FILE out; const: @(LmP0Node node); const: @(char path); int: in_l1; int: depth) int
    end: prototype
"""


def main() -> None:
    lines = SRC.read_text(encoding="utf-8").splitlines()
    start = 0
    for i, line in enumerate(lines):
        if line.startswith("include:"):
            start = i
            break
    body: list[str] = []
    for line in lines[start:]:
        s = line.strip()
        if s.startswith("predef:"):
            continue
        body.append("    " + pp.convert_line(line) if line.strip() else "")
    out: list[str] = []
    out.append("L1:")
    out.append('    predef: "l1src/parser.lm2"')
    out.extend(body)
    first_fn = next((i for i, line in enumerate(out) if line.strip().startswith("fn:")), len(out))
    proto_lines = [ln.rstrip() for ln in EMIT_PROTOS.strip("\n").splitlines()]
    proto_lines.append("")
    out[first_fn:first_fn] = proto_lines
    if not out[-1].strip().startswith("end:"):
        out.append("end: L1")
    else:
        # last end: external — still need end: L1
        if "end: L1" not in out[-3:]:
            out.append("end: L1")
    out.append("")
    DST.write_text("\n".join(out), encoding="utf-8", newline="\n")
    print(f"wrote {DST} lines={len(out)}")


if __name__ == "__main__":
    main()
