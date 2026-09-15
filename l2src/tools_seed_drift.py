import io, re, sys

def funcs(path):
    out = {}
    name = None
    body = []
    for line in io.open(path, encoding='utf-8', newline='').read().split('\n'):
        m = re.match(r'^fn: ([A-Za-z_][A-Za-z0-9_]*) ', line)
        if m:
            if name:
                out[name] = body
            name = m.group(1)
            body = []
            continue
        if name is not None:
            if line.startswith('fn: ') or (line.startswith('end: ') and not line.startswith('end: if')):
                out[name] = body
                name = None
                body = []
            else:
                body.append(line)
    if name:
        out[name] = body
    return out

def calls(body):
    s = '\n'.join(body)
    return set(re.findall(r'\b(l1_[a-z0-9_]+|lm_p0_[a-z0-9_]+)\s*\(', s))

cur = funcs('l1src/l1trans.lm1')
seed = funcs('stg/l1_baseline/lm2/l1trans.lm2')

only_cur = sorted(set(cur) - set(seed))
only_seed = sorted(set(seed) - set(cur))
print('functions only in the CURRENT source (%d):' % len(only_cur))
for n in only_cur: print('   ', n)
print('functions only in the SEED (%d):' % len(only_seed))
for n in only_seed: print('   ', n)

print('\ncommon functions whose CALL SET differs:')
diffs = 0
for n in sorted(set(cur) & set(seed)):
    a, b = calls(cur[n]), calls(seed[n])
    if a != b:
        diffs += 1
        print('   %-34s current-only=%s  seed-only=%s' % (n, sorted(a-b) or '-', sorted(b-a) or '-'))
print('   (%d functions differ in what they call)' % diffs)
