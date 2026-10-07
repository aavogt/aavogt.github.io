#!/usr/bin/env python3
# usage: run from the project root:  ./explicit.py src/Foo.hs out/Foo.hs
import json, subprocess, sys, pathlib

src = pathlib.Path(sys.argv[1]).resolve()
dst = pathlib.Path(sys.argv[2])
text = src.read_text()
uri = src.as_uri()
root = pathlib.Path.cwd()

proc = subprocess.Popen(["haskell-language-server-wrapper", "--lsp"],
                        stdin=subprocess.PIPE, stdout=subprocess.PIPE, cwd=root)

def send(msg):
    body = json.dumps(msg).encode()
    proc.stdin.write(b"Content-Length: %d\r\n\r\n" % len(body) + body)
    proc.stdin.flush()

def recv():
    n = 0
    while True:
        line = proc.stdout.readline().strip()
        if not line:
            break
        if line.lower().startswith(b"content-length"):
            n = int(line.split(b":")[1])
    return json.loads(proc.stdout.read(n))

_id = 0
def request(method, params):
    global _id
    _id += 1
    my = _id
    send({"jsonrpc": "2.0", "id": my, "method": method, "params": params})
    while True:
        m = recv()
        if "method" in m and "id" in m:          # server -> client request: acknowledge
            res = [None] * len(m["params"].get("items", [])) if m["method"] == "workspace/configuration" else None
            send({"jsonrpc": "2.0", "id": m["id"], "result": res})
        elif m.get("id") == my:
            return m.get("result")

def notify(method, params):
    send({"jsonrpc": "2.0", "method": method, "params": params})

request("initialize", {
    "processId": None, "rootUri": root.as_uri(),
    "capabilities": {"workspace": {"workspaceEdit": {"documentChanges": True},
                                   "configuration": True},
                     "textDocument": {"codeAction": {
                         "resolveSupport": {"properties": ["edit"]},
                         "codeActionLiteralSupport": {"codeActionKind": {"valueSet": ["", "quickfix", "refactor", "source"]}}}}}})
notify("initialized", {})
notify("textDocument/didOpen", {"textDocument": {"uri": uri, "languageId": "haskell", "version": 1, "text": text}})

lines = text.splitlines(keepends=True)
actions = request("textDocument/codeAction", {
    "textDocument": {"uri": uri},
    "range": {"start": {"line": 1, "character": 0}, "end": {"line": 1, "character": 0}},
    "context": {"diagnostics": []}}) or []

act = next((a for a in actions if "Make all imports explicit" in a["title"]), None)
if act is None:
    sys.exit("action not offered; titles seen: " + str([a["title"] for a in actions]))
if "edit" not in act:
    act = request("codeAction/resolve", act)

edit = act["edit"]
if "documentChanges" in edit:
    edits = [e for dc in edit["documentChanges"] for e in dc["edits"]]
else:
    edits = edit["changes"][uri]

starts = [0]
for l in lines:
    starts.append(starts[-1] + len(l))
off = lambda p: starts[min(p["line"], len(lines))] + p["character"]   # assumes BMP text

for e in sorted(edits, key=lambda e: off(e["range"]["start"]), reverse=True):
    text = text[:off(e["range"]["start"])] + e["newText"] + text[off(e["range"]["end"]):]

dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(text)

request("shutdown", None)
notify("exit", None)
