import re, glob, sys
D = sys.argv[1]
func_re = re.compile(r'^DEFINE_REX_FUNC\(sub_([0-9A-F]{8})\)')
label_re = re.compile(r'^loc_([0-9A-F]{8}):')
instr_re = re.compile(r'^\t// (\S+)')
funcs = {}
for path in glob.glob(D + '/FancyPants_recomp.*.cpp'):
    cur = None
    for line in open(path, errors='replace'):
        m = func_re.match(line)
        if m:
            cur = int(m.group(1), 16); pc = cur; hi = cur; last = None; lastpc = cur
            funcs[cur] = [hi, last]
            continue
        if cur is None: continue
        m = label_re.match(line)
        if m:
            pc = int(m.group(1), 16); continue
        m = instr_re.match(line)
        if m:
            pc += 4
            if pc > funcs[cur][0]:
                funcs[cur] = [pc, m.group(1)]
starts = sorted(set(int(x, 16) for x in re.findall(r'sub_([0-9A-F]{8})', open(D + '/FancyPants_funcs.h').read())))
code_end = 0x824981CC
for i, s in enumerate(starts):
    nxt = starts[i + 1] if i + 1 < len(starts) else code_end
    if s not in funcs: continue
    end, last = funcs[s]
    if nxt - end >= 8:
        print(f'{s:08X} {end:08X} {nxt:08X} {nxt-end:6d} {last}')
