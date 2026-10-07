#!/usr/bin/env python3
"""Make hscolour annotation files for the original *.hs files.

The names come from explicit_imports/<same name>.hs.
The docs come from one ghci session loaded with the first original file.
"""
import glob, os, re, subprocess, sys, json

SRC_DIR   = "."
EXPL_DIR  = "explicit_imports"
ANN_TYPE  = "doc"          # the annotation_type line
END_EXCL  = 1              # 1: end column is one past the last char, 0: inclusive
MARK      = "@@DOC@@"

def import_names(path):
    """Identifiers inside explicit (non-hiding) import lists."""
    text = open(path, encoding="utf-8").read()
    names = set()
    for m in re.finditer(r'^import\b(?:[^(\n]|\n[ \t])*\(', text, re.M):
        if re.search(r'\bhiding\s*\($', m.group(0)):
            continue
        j, depth = m.end(), 1
        while depth:
            depth += (text[j] == '(') - (text[j] == ')')
            j += 1
        body = text[m.end():j - 1]
        names |= set(re.findall(r"[A-Za-z_][\w']*", body)) - {"type", "pattern"}
    return names

SYM = r"!#$%&*+./<=>?@\\^|~:-"
LINE_COMMENT = re.compile(r"-{2,}(?![%s])[^\n]*" % re.escape(SYM).replace(r"\-", "-"))
STRING = re.compile(r'"(?:[^"\\\n]|\\.|\\\n\s*\\)*"', re.S)
CHAR   = re.compile(r"'(?:[^'\\]|\\[^']+)'")
IDENT  = re.compile(r"[A-Za-z_][\w']*")

def occurrences(text, names):
    """Yield (line, col, end_col, name), 1-based, skipping comments/strings/import lines."""
    starts = [0] + [m.end() for m in re.finditer(r'\n', text)]
    import bisect
    i, n = 0, len(text)
    while i < n:
        if text.startswith('{-', i):                      # nested block comment
            d = 0
            while i < n:
                if text.startswith('{-', i): d += 1; i += 2
                elif text.startswith('-}', i):
                    d -= 1; i += 2
                    if d == 0: break
                else: i += 1
            continue
        for rx in (LINE_COMMENT, STRING):
            m = rx.match(text, i)
            if m: i = m.end(); break
        else:
            m = IDENT.match(text, i)
            if m:
                ln = bisect.bisect_right(starts, i) - 1
                line_text = text[starts[ln]:text.find('\n', starts[ln]) if '\n' in text[starts[ln]:] else n]
                if m.group(0) in names and not line_text.startswith("import"):
                    c = i - starts[ln] + 1
                    yield ln + 1, c, c + len(m.group(0)) - 1 + END_EXCL, m.group(0)
                i = m.end()
            else:
                m = CHAR.match(text, i)
                i = m.end() if m else i + 1

def docs(path, names):
    cmds = [':set prompt ""', ':set prompt-cont ""']
    for nm in sorted(names):
        cmds += [f'putStrLn "{MARK}{nm}"', f':doc {nm}']
    cmds.append(":q")
    out = subprocess.run(["ghci", path], input="\n".join(cmds) + "\n",
                         capture_output=True, text=True).stdout
    res, cur = {}, None
    for line in out.splitlines():
        if line.startswith(MARK):
            cur = line[len(MARK):]; res[cur] = []
        elif cur is not None:
            res[cur].append(line.rstrip())
    return {k: "\n".join(v).strip("\n") for k, v in res.items() if "".join(v).strip()}

def main():
    originals = sorted(glob.glob(os.path.join(SRC_DIR, "*.hs")))
    if not originals: sys.exit("no *.hs files found")
    per_file = {}
    for f in originals:
        e = os.path.join(EXPL_DIR, os.path.basename(f))
        if os.path.exists(e):
            per_file[f] = import_names(e)
    all_names = set().union(*per_file.values())
    d = docs(originals[0], all_names)          # no name conflicts: one session serves all
    for f, names in per_file.items():
        text = open(f, encoding="utf-8").read()
        blocks = [f"{l}:{c}-{l}:{e}\n{ANN_TYPE}\n{d[nm]}\n\n"
                  for l, c, e, nm in occurrences(text, names) if nm in d]
        out = os.path.splitext(f)[0] + ".annot"
        open(out, "w", encoding="utf-8").write("\n".join(blocks))
        items = [(l, c, e, d[nm]) for l, c, e, nm in occurrences(text, names) if nm in d]
        json.dump(items, open(os.path.splitext(f)[0] + ".annot.json", "w"))
        print(f"{out}: {len(blocks)} annotations")

if __name__ == "__main__":
    main()
