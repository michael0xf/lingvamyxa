from pathlib import Path
p = Path(r"C:\Nyasha_Planet\lingvamyxa\mixa_manager\tests\mixa_ingress_host_harness.c")
t = p.read_text(encoding="utf-8")
old = "int main(void)\n{"
new = (
    "int main(int argc, char **argv)\n"
    "{\n"
    "    int ai;\n"
    "    for (ai = 1; ai < argc; ai++) {\n"
    "        if (argv[ai] != 0 && strcmp(argv[ai], \"--force-join-timeout\") == 0) {\n"
    "            return run_force_join_timeout();\n"
    "        }\n"
    "    }\n"
)
if old not in t:
    raise SystemExit("main missing")
t = t.replace(old, new, 1)
p.write_text(t, encoding="utf-8", newline="\n")
print("main patched")
