import base64, pathlib, sys
scratch = pathlib.Path(r"C:\Nyasha_Planet\lingvamyxa\work_chat\grok_bot\_ticket_scratch")
scratch.mkdir(parents=True, exist_ok=True)
# argv: stem dest nchunks
stem, dest, n = sys.argv[1], sys.argv[2], int(sys.argv[3])
parts = []
for i in range(n):
    parts.append((scratch / f"{stem}.{i:03d}.b64").read_text(encoding="ascii"))
raw = base64.b64decode("".join(parts))
pathlib.Path(dest).write_bytes(raw)
print("wrote", dest, len(raw))
