# Convert frozen 620db86 parser.lm2 C-like functions into L1.
# Skips L2 tables, registry/MessageThread ontology, and selftest.
# Run from repo root. Output: wrapperless l1src/parser.lm1

from __future__ import annotations

import re
from pathlib import Path

SRC = Path(r"C:\Nyasha_Planet\lingvamyxa_old_worked_version\lm2\parser.lm2")
DST = Path(r"C:\Nyasha_Planet\lingvamyxa\l1src\parser.lm1")

PTR = {
    "LmP0Text", "LmP0Span", "LmP0Diagnostic", "LmP0Structure", "LmP0Trailer",
    "LmP0Frame", "LmP0NodeAs", "LmP0Node", "LmP0Field", "LmP0Document",
    "LmP0StreamEvent", "LmP0PendingDelimiter", "LmP0PendingMix",
    "LmP0IndentStack", "LmP0DisabledState", "LmP0Stack", "LmP0Dump",
    "LmP0DumpFrame", "LmP0FieldParseLoopFrame", "LmP0PostprocessFrame",
    "LmOwnPtrStack", "LmOwnArena", "FILE",
}

VAL = {
    "int", "void", "char", "size_t", "unsigned", "long", "short", "uchar",
    "LmP0NodeKind", "LmP0FrameFlags", "LmP0NodeFlags", "LmP0TrailerFlags",
    "LmP0StreamEventKind", "LmP0TrailerRole", "LmP0DashFenceStatus",
    "LmP0FieldParseFlags", "LmP0FieldParseLoopContinuation",
    "LmOwnDestroyFields", "LmOwnDelete",
}

HAND = {
    "lm_p0_document_init_owners",
    "lm_p0_document_owners_belong_to_actor",
    "lm_p0_document_destroy_owners",
    "lm_p0_document_freeze_tree",
    "lm_p0_scan_registry_compact_atom_piece",
    "lm_p0_node_kind_class_name",
    "lm_p0_trailer_role",
}

SKIP_SUBSTR = (
    "registry",
    "lm_message_thread",
    "LmTable",
    "LmRegistry",
)

DECL_RE = re.compile(
    r"^(\s*)(const:\s+)?(?:\(([A-Za-z_][A-Za-z0-9_]*):\s+([A-Za-z_][A-Za-z0-9_]*)\)"
    r"|([A-Za-z_][A-Za-z0-9_]*):\s+([A-Za-z_][A-Za-z0-9_]*))(\s+.*)?$"
)


def fn_name(line: str) -> str | None:
    m = re.match(r"^(?:external:\s*)?(?:fn|sub):\s+([A-Za-z_][A-Za-z0-9_]*)", line.strip())
    return m.group(1) if m else None


def should_skip_name(name: str) -> bool:
    if name in HAND:
        return True
    low = name.lower()
    return any(s.lower() in low for s in SKIP_SUBSTR)


def convert_type_frag(s: str) -> str:
    def repl_const_paren(m: re.Match) -> str:
        t, n = m.group(1), m.group(2)
        if t in PTR:
            return f"const: @({t} {n})"
        return f"const: {t}: {n}"

    def repl_paren(m: re.Match) -> str:
        t, n = m.group(1), m.group(2)
        if t in PTR:
            return f"(@: {t} {n})"
        return f"({t}: {n})"

    s = re.sub(
        r"const:\s+\(([A-Za-z_][A-Za-z0-9_]*):\s+([A-Za-z_][A-Za-z0-9_]*)\)",
        repl_const_paren,
        s,
    )
    def upgrade_out_ptr(m: re.Match) -> str:
        t, n = m.group(1), m.group(2)
        if t in PTR:
            return f"@@: {t} {n}"
        return m.group(0)

    s = re.sub(
        r"(?<!@)@: ([A-Za-z_][A-Za-z0-9_]*) ([A-Za-z_][A-Za-z0-9_]*)",
        upgrade_out_ptr,
        s,
    )
    s = re.sub(
        r"\(([A-Za-z_][A-Za-z0-9_]*):\s+([A-Za-z_][A-Za-z0-9_]*)\)",
        repl_paren,
        s,
    )
    s = re.sub(
        r"\(cast:\s+\(@:\s+([A-Za-z_][A-Za-z0-9_]*)\)",
        lambda m: f"(cast: (@@: {m.group(1)})" if m.group(1) in PTR else m.group(0),
        s,
    )
    s = re.sub(
        r"\(cast:\s+\(([A-Za-z_][A-Za-z0-9_]*)\)",
        lambda m: f"(cast: (@: {m.group(1)})" if m.group(1) in PTR else m.group(0),
        s,
    )

    def repl_bare(m: re.Match) -> str:
        lead, t, n = m.group(1), m.group(2), m.group(3)
        if t in ("fn", "sub", "end", "if", "else", "while", "for", "return"):
            return m.group(0)
        if t in PTR:
            return f"{lead}@: {t} {n}"
        return m.group(0)

    s = re.sub(
        r"(^|[\s;(])([A-Za-z_][A-Za-z0-9_]*):\s+([A-Za-z_][A-Za-z0-9_]*)",
        repl_bare,
        s,
    )
    return s


def convert_return(s: str) -> str:
    m = re.search(r"\)\s+(const:\s+)?(@{1,2}:\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*$", s)
    if not m:
        return s
    const, already_ptr, t = m.group(1), m.group(2), m.group(3)
    stars = 0
    if already_ptr:
        stars = 2 if already_ptr.strip().startswith("@@") else 1
    if t in PTR:
        stars += 1
    if stars <= 0:
        if const and t in PTR:
            return s[:m.start()] + f") const: @: {t}"
        return s
    at = "@@:" if stars >= 2 else "@:"
    prefix = f"const: {at}" if const else at
    return s[:m.start()] + f") {prefix} {t}"


def convert_line(line: str) -> str:
    raw = line.rstrip("\n")
    if raw.strip().startswith("fn:") or raw.strip().startswith("sub:"):
        return convert_return(convert_type_frag(raw))
    m = DECL_RE.match(raw)
    if m:
        indent, const, t1, n1, t2, n2, rest = m.groups()
        t = t1 or t2
        n = n1 or n2
        rest = rest or ""
        if t in PTR:
            c = "const: " if const else ""
            return f"{indent}{c}@: {t} {n}{rest}"
        if t in VAL:
            return raw
    raw = raw.replace("lm_own_arena_init(", "lm_own_arena_init_unused_thread(")
    raw = raw.replace("c.lm_lmx_message_thread", "0")
    raw = convert_type_frag(raw)
    return raw


def is_skip_line(s: str) -> bool:
    t = s.strip()
    return (
        t.startswith("table:")
        or t.startswith("import:")
        or t.startswith("ifdef:")
        or t.startswith("ifndef")
        or t.startswith("predef:")
        or t.startswith("`ifndef")
        or t.startswith("include")
        or t.startswith("define")
        or t.startswith("os")
        or t.startswith("forward")
        or t.startswith("alias")
        or t.startswith("constant")
        or t.startswith("layout")
        or t.startswith("prototype")
        or t.startswith("unit")
        or t == "fn"
        or t == "end: ifdef"
    )


HAND_PROTOS = r'''
        fn: lm_p0_document_init_owners (@: LmP0Document document) int
        fn: lm_p0_document_owners_belong_to_actor (const: @(LmP0Document document)) int
        sub: lm_p0_document_destroy_owners (@: LmP0Document document)
        sub: lm_p0_document_freeze_tree (@: LmP0Document document)
        fn: lm_p0_scan_registry_compact_atom_piece (const: @(char text); size_t: end_index; size_t: start) size_t
        fn: lm_p0_registry_load_default () int
        fn: lm_p0_registry_table_has_rows (const: @(char table)) int
        fn: lm_p0_registry_table_has_rows_loaded_or_loading (const: @(char table)) int
        fn: lm_p0_registry_lookup_cstr (const: @(char key); const: @(char table)) const: @: char
        fn: lm_p0_registry_compare_enabled () int
        fn: lm_p0_registry_lookup_key_by_unsigned_payload (const: @(char table); unsigned: value) const: @: char
        fn: lm_p0_trailer_role (const: @(char text); size_t: length) LmP0TrailerRole
        fn: lm_p0_node_kind_class_name (LmP0NodeKind: kind) const: @: char
'''

HAND_BLOCK = r'''
    fn: lm_p0_document_init_owners (@: LmP0Document document) int
        if: document = 0
            return: 1
        if: document\owners_initialized != 0
            return: 0
        document\source_owner: lm_own_new_zero(c.sizeof(c.LmOwnArena))
        document\token_arena: lm_own_new_zero(c.sizeof(c.LmOwnArena))
        document\tree_arena: lm_own_new_zero(c.sizeof(c.LmOwnArena))
        document\diagnostic_arena: lm_own_new_zero(c.sizeof(c.LmOwnArena))
        if: document\source_owner = 0 || document\token_arena = 0 || document\tree_arena = 0 || document\diagnostic_arena = 0
            lm_own_delete(document\diagnostic_arena, 0)
            lm_own_delete(document\tree_arena, 0)
            lm_own_delete(document\token_arena, 0)
            lm_own_delete(document\source_owner, 0)
            document\diagnostic_arena: 0
            document\tree_arena: 0
            document\token_arena: 0
            document\source_owner: 0
            return: 1
        document\owners_initialized: 1
        if: lm_own_arena_init(document\source_owner) != 0 || lm_own_arena_init(document\token_arena) != 0 || lm_own_arena_init(document\tree_arena) != 0 || lm_own_arena_init(document\diagnostic_arena) != 0
            lm_p0_document_destroy_owners(document)
            return: 1
        document\diagnostic: lm_own_arena_new_zero(document\diagnostic_arena, c.sizeof(c.LmP0Diagnostic))
        if: document\diagnostic = 0
            lm_p0_document_destroy_owners(document)
            return: 1
    return: 0

    fn: lm_p0_document_owners_belong_to_actor (const: @(LmP0Document document)) int
    return: 1

    sub: lm_p0_document_destroy_owners (@: LmP0Document document)
        if: document != 0 && document\owners_initialized != 0
            lm_own_arena_destroy(document\diagnostic_arena)
            lm_own_delete(document\diagnostic_arena, 0)
            lm_own_arena_destroy(document\tree_arena)
            lm_own_delete(document\tree_arena, 0)
            lm_own_arena_destroy(document\token_arena)
            lm_own_delete(document\token_arena, 0)
            lm_own_arena_destroy(document\source_owner)
            lm_own_delete(document\source_owner, 0)
            document\diagnostic_arena: 0
            document\tree_arena: 0
            document\token_arena: 0
            document\source_owner: 0
            document\diagnostic: 0
            document\owners_initialized: 0
            document\frozen: 0
        ---
    end: lm_p0_document_destroy_owners

    sub: lm_p0_document_freeze_tree (@: LmP0Document document)
        if: document != 0 && document\owners_initialized != 0
            lm_own_arena_freeze(document\tree_arena)
            lm_own_arena_freeze(document\source_owner)
            document\frozen: 1
        ---
    end: lm_p0_document_freeze_tree

    fn: lm_p0_scan_registry_compact_atom_piece (const: @(char text); size_t: end_index; size_t: start) size_t
        size_t: best
        if: start >= end_index
            return: start
        if: (start + 1U) >= end_index
            return: start + 1U
        best: 0U
        if: text[start] = 33 && text[start + 1U] = 61
            best: 2U
        if: text[start] = 60 && text[start + 1U] = 61
            best: 2U
        if: text[start] = 62 && text[start + 1U] = 61
            best: 2U
        if: text[start] = 38 && text[start + 1U] = 38
            best: 2U
        if: text[start] = 124 && text[start + 1U] = 124
            best: 2U
        if: text[start] = 43 && text[start + 1U] = 43
            best: 2U
        if: text[start] = 45 && text[start + 1U] = 45
            best: 2U
        if: text[start] = 91 && text[start + 1U] = 93
            best: 2U
        if: text[start] = 60 && text[start + 1U] = 60
            best: 2U
        if: text[start] = 62 && text[start + 1U] = 62
            best: 2U
        if: best > 0U
            return: start + best
    return: start + 1U

    fn: lm_p0_registry_load_default () int
    return: 0

    fn: lm_p0_registry_table_has_rows (const: @(char table)) int
    return: 0

    fn: lm_p0_registry_table_has_rows_loaded_or_loading (const: @(char table)) int
    return: 0

    fn: lm_p0_registry_lookup_cstr (const: @(char key); const: @(char table)) const: @: char
    return: 0

    fn: lm_p0_registry_compare_enabled () int
    return: 0

    fn: lm_p0_registry_lookup_key_by_unsigned_payload (const: @(char table); unsigned: value) const: @: char
    return: 0

    fn: lm_p0_trailer_role (const: @(char text); size_t: length) LmP0TrailerRole
        if: lm_p0_dash_fence_status_after_comment_trim(text, length, 0) = c.LM_P0_DASH_FENCE_VALID
            return: c.LM_P0_TRAILER_ROLE_DASH_CUTTER
    return: lm_p0_legacy_trailer_role(text, length)

    fn: lm_p0_node_kind_class_name (LmP0NodeKind: kind) const: @: char
        if: kind = c.LM_P0_NODE_STRUCTURE
            return: "structure"
        if: kind = c.LM_P0_NODE_FRAME
            return: "frame"
        if: kind = c.LM_P0_NODE_ATOM
            return: "atom"
        if: kind = c.LM_P0_NODE_DISABLED
            return: "disabled"
    return: "unknown"
'''


def split_functions(lines: list[str]) -> list[list[str]]:
    chunks: list[list[str]] = []
    i = 0
    n = len(lines)
    while i < n:
        s = lines[i]
        if s.strip().startswith("`ifndef") or "ifndef-default" in s:
            i += 1
            continue
        if s.strip().startswith("table:") or s.strip().startswith("ifdef:"):
            depth = 0
            while i < n:
                if lines[i].strip().startswith("table:") or lines[i].strip().startswith("ifdef:"):
                    depth += 1
                if lines[i].strip().startswith("end:"):
                    depth -= 1
                    i += 1
                    if depth <= 0:
                        break
                    continue
                i += 1
            continue
        if s.strip().startswith("external:"):
            chunk = [s]
            i += 1
            depth = 1
            while i < n and depth > 0:
                chunk.append(lines[i])
                if lines[i].strip().startswith("external:"):
                    depth += 1
                if lines[i].strip() == "end: external":
                    depth -= 1
                i += 1
            chunks.append(chunk)
            continue
        name = fn_name(s)
        if name:
            chunk = [s]
            i += 1
            while i < n:
                nxt = lines[i]
                ns = nxt.strip()
                if ns.startswith("fn:") or ns.startswith("sub:") or ns.startswith("external:") or ns.startswith("table:") or ns.startswith("ifdef:") or ns.startswith("import:") or "ifndef-default" in ns:
                    break
                chunk.append(nxt)
                i += 1
            chunks.append(chunk)
            continue
        i += 1
    return chunks


def chunk_name(chunk: list[str]) -> str | None:
    for line in chunk:
        n = fn_name(line)
        if n:
            return n
    return None


def main() -> None:
    text = SRC.read_text(encoding="utf-8")
    lines = text.splitlines()
    start = 0
    for i, line in enumerate(lines):
        if line.startswith("fn: lm_p0_document_init_owners"):
            start = i
            break
    chunks = split_functions(lines[start:])
    kept: list[str] = []
    protos: list[str] = []
    for chunk in chunks:
        name = chunk_name(chunk)
        if name is None:
            continue
        if should_skip_name(name):
            continue
        converted = [convert_line(x) for x in chunk]
        while converted and converted[-1].strip() == "":
            converted.pop()
        sig = None
        for line in converted:
            st = line.strip()
            if st.startswith("fn:") or st.startswith("sub:"):
                sig = st
                break
        if sig:
            protos.append("        " + sig)
        for line in converted:
            if line.strip() == "end: external":
                kept.append("    end: external")
                continue
            if line.strip() == "external:":
                kept.append("    external:")
                continue
            kept.append("    " + line if line.strip() else "")
        kept.append("")

    def dedent4(s: str) -> str:
        lines = s.splitlines()
        out_lines = [ln[4:] if ln.startswith("    ") else ln for ln in lines]
        return "\n".join(out_lines)

    out: list[str] = []
    out.append('predef: "l1src/own.lm1"')
    out.append('predef: "l1src/parser_text.lm1"')
    out.append('include: "l1src/p0.h" "<stdio.h>" "<stdlib.h>" "<string.h>" "<ctype.h>" "<limits.h>"')
    out.append("")
    out.append("prototype:")
    out.append(dedent4(HAND_PROTOS.rstrip()))
    for p in protos:
        out.append(p[4:] if p.startswith("    ") else p)
    out.append("end: prototype")
    out.append("")
    out.append(dedent4(HAND_BLOCK.rstrip()))
    out.append("")
    out.extend([(ln[4:] if ln.startswith("    ") else ln) for ln in kept])
    out.append("")
    text_out = "\n".join(out)
    old_mix = (
        "                            node\\as\\structure: payload_document\\root\\as\\structure\n"
        "                            payload_document\\root\\as\\structure: 0\n"
        "                            if: lm_own_arena_absorb(document\\tree_arena, payload_document\\tree_arena) != 0\n"
        "                                lm_p0_set_diagnostic(document, 1, span_line, span_column, \"out of memory while moving MIX tree into parser arena\")\n"
        "                            else:\n"
        "                                lm_p0_adjust_structure_spans_to_document(document, node\\as\\structure, payload_offset)\n"
        "                                status: 1"
    )
    new_mix = (
        "                            if: lm_own_arena_absorb(document\\tree_arena, payload_document\\tree_arena) != 0\n"
        "                                lm_p0_set_diagnostic(document, 1, span_line, span_column, \"out of memory while moving MIX tree into parser arena\")\n"
        "                            else:\n"
        "                                node\\as\\structure: payload_document\\root\\as\\structure\n"
        "                                payload_document\\root\\as\\structure: 0\n"
        "                                lm_p0_adjust_structure_spans_to_document(document, node\\as\\structure, payload_offset)\n"
        "                                status: 1"
    )
    mix_hits = text_out.count(old_mix)
    if mix_hits != 1 and new_mix not in text_out:
        raise SystemExit(f"MIX absorb reorder: expected 1 frozen site, found {mix_hits}")
    if mix_hits == 1:
        text_out = text_out.replace(old_mix, new_mix)
    DST.write_text(text_out, encoding="utf-8", newline="\n")
    print(f"wrote {DST} functions={len(protos)} lines={len(out)}")


if __name__ == "__main__":
    main()
