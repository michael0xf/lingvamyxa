from pathlib import Path
import re
p = Path(r"C:\Nyasha_Planet\lingvamyxa\mixa_manager\tests\mixa_ingress_host_harness.c")
t = p.read_text(encoding="utf-8")
pairs = [
(
    "    if (join_checked(&th, \"foreign_poster\") != 0) {\n        fail = 1;\n        goto fail_keep_runtime;\n    }\n",
    "    if (join_checked(&th, \"foreign_poster\") != 0) {\n        fail = 1;\n        return exit_fail_keep_runtime(owner_tid, \"foreign_poster_join\");\n    }\n",
),
(
    "    if (late_th == 0) {\n        fprintf(stderr, \"late CreateThread failed\\n\");\n        fail = 1;\n        goto fail_keep_runtime;\n    }\n",
    "    if (late_th == 0) {\n        fprintf(stderr, \"late CreateThread failed\\n\");\n        fail = 1;\n        return exit_fail_keep_runtime(owner_tid, \"late_CreateThread\");\n    }\n",
),
(
    "    if (join_checked(&late_th, \"foreign_late_poster\") != 0) {\n        fail = 1;\n        goto fail_keep_runtime;\n    }\n",
    "    if (join_checked(&late_th, \"foreign_late_poster\") != 0) {\n        fail = 1;\n        return exit_fail_keep_runtime(owner_tid, \"foreign_late_poster_join\");\n    }\n",
),
]
for old, new in pairs:
    if old not in t:
        raise SystemExit("missing:\n" + repr(old))
    t = t.replace(old, new, 1)
pat = (
    r"    lmx_msg_host_drain\(g_rt\);\n"
    r"    lmx_msg_runtime_delete\(g_rt\);\n"
    r"    g_rt = 0;\n"
    r"    goto write_evidence;\n\n"
    r"fail_keep_runtime:\n"
    r"    fprintf\(stderr, \"quiescence incomplete .{1,3} leaving runtime \(no delete while poster reachable\)\\n\"\);\n"
    r"    /\* Do not CloseHandle live threads; do not delete g_rt\. \*/\n\n"
    r"write_evidence:\n"
    r"    if \(g_posted_event != 0\) \{"
)
repl = (
    "    lmx_msg_host_drain(g_rt);\n"
    "    lmx_msg_runtime_delete(g_rt);\n"
    "    g_rt = 0;\n\n"
    "    /* write_evidence: only after quiescence completed (joins succeeded). */\n"
    "    if (g_posted_event != 0) {"
)
nt, n = re.subn(pat, repl, t, count=1)
if n != 1:
    idx = t.find("goto write_evidence")
    raise SystemExit("tail re fail n=%d near=%r" % (n, t[idx:idx+280] if idx >= 0 else None))
t = nt
p.write_text(t, encoding="utf-8", newline="\n")
print("ok gotos=%d labels=%d" % (t.count("goto fail_keep_runtime"), t.count("fail_keep_runtime:")))
