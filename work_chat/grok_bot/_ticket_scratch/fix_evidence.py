from pathlib import Path
p = Path(r"C:\Nyasha_Planet\lingvamyxa\mixa_manager\tests\mixa_file_selftest.lm1")
t = p.read_text(encoding="utf-8")
t = t.replace("    @: void ev\n", "", 1)
if "[]: char eline 256" not in t:
    t = t.replace("    []: size_t sz 1\n", "    []: size_t sz 1\n    []: char eline 256\n", 1)
start = t.find("    ev:")
if start < 0:
    # maybe already partially patched
    start = t.find("    # Evidence via the seam")
    if start >= 0:
        print("already evidence via seam")
        raise SystemExit(0)
    raise SystemExit("ev block start missing")
end = t.find("    if: failures != 0", start)
if end < 0:
    raise SystemExit("end missing")
new = '''    # Evidence via the seam under test (append), not stdio FILE locals.
    out[0]: 0
    st: mixa_file_open(out, "build/mixa/logs/mixa_file_selftest.evidence.txt", MIXA_FILE_MODE_APPEND)
    if: st = MIXA_FILE_OK && out[0] != 0
        wrote[0]: 0U
        c.snprintf(eline, 256U, "file_seam_3_5=1 failures=%d nul_crlf=1 ownership_close=1 missing_checked=1 nonascii=1\\n", failures)
        mixa_file_append(out[0], eline, c.strlen(eline), wrote)
        c.snprintf(eline, 256U, "ops=open,close,append,read,seek,size translator_pin=stable65D5\\n")
        wrote[0]: 0U
        mixa_file_append(out[0], eline, c.strlen(eline), wrote)
        mixa_file_close(out[0])
    ---

'''
t = t[:start] + new + t[end:]
p.write_text(t, encoding="utf-8", newline="\n")
print("evidence via seam ok")
