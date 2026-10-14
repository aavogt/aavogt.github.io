#!/usr/bin/env python3
"""Build an HTML index of the Haskell annotations by usage frequency."""
from collections import Counter, defaultdict
from html import escape
import json
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import quote

MODULE = re.compile(r"--\s*(?:Identifier|Type constructor) defined in [‘'](.+?)[’']")
HADDOCK_MARKUP = re.compile(r"@([^@\n]+)@|'([A-Za-z_][\w'.]*)'|\"([A-Z][\w.]*)\"")
TAGS = Path.home() / "wip/rapids/tags"


def parse_annotation(text):
    lines = text.splitlines()
    if lines[0].strip() == "<has no documentation>":
        return None
    signature = lines[0].strip()
    module = None
    header_module = MODULE.search(signature)
    if header_module:
        module = header_module.group(1)
        signature = signature[:header_module.start()].rstrip()

    body = []
    for line in lines[1:]:
        line = line.strip()
        match = MODULE.search(line)
        if match:
            module = match.group(1)
        elif line.startswith("-- |"):
            body.append(line[4:].lstrip())
        elif line == "--":
            body.append("")
        elif line.startswith("--"):
            body.append(line[2:].lstrip())
    name, separator, _ = signature.partition(" :: ")
    if not separator:
        raise ValueError(f"annotation has no type signature: {text!r}")
    return name, signature, module, "\n".join(body).strip()


def rst_source(text):
    # Haddock examples use leading > prompts; preserve each run as a Pandoc
    # literal block instead of letting reStructuredText parse it as prose.
    def literal(match):
        value = next(group for group in match.groups() if group is not None)
        return f"``{value}``"

    output, prose = [], []

    def flush_prose():
        if prose:
            source = HADDOCK_MARKUP.sub(literal, "\n".join(prose))
            output.extend(source.split("\n"))
            prose.clear()

    lines = text.splitlines()
    i = 0
    while i < len(lines):
        if not lines[i].lstrip().startswith(">"):
            prose.append(lines[i])
            i += 1
            continue
        flush_prose()
        while output and output[-1] == "":
            output.pop()
        output.extend(["", "::", ""])
        while i < len(lines) and lines[i].strip():
            output.append("    " + lines[i])
            i += 1
        output.append("")
    flush_prose()
    return "\n".join(output).strip()


def render_rst(text):
    if not text or text == "<has no documentation>":
        return ""
    rendered = subprocess.run(
        ["pandoc", "--from=rst", "--to=html", "--wrap=none"],
        input=rst_source(text), text=True, capture_output=True, check=True,
    ).stdout.strip()
    return rendered


def load_source_tags(names):
    tags = defaultdict(dict)
    if not TAGS.is_file():
        return tags
    with TAGS.open(encoding="utf-8") as source:
        for line in source:
            name, separator, entry = line.rstrip("\n").partition("\t")
            if not separator or name not in names:
                continue
            path, separator, line_number = entry.partition("\t")
            if separator and path.startswith("lib/Rapids/") and line_number.isdecimal():
                tags[name][path] = line_number
    return tags


def defined_in_url(name, module, tags):
    if module.startswith("Rapids."):
        path = f"lib/{module.replace('.', '/')}.hs"
        url = f"https://github.com/aavogt/rapids/blob/main/{quote(path, safe='/')}"
        line_number = tags.get(name, {}).get(path)
        return f"{url}#L{line_number}" if line_number else url

    package, separator, module_name = module.partition(":")
    if not separator:
        return None
    module_path = quote(module_name.replace(".", "-"), safe="-")
    anchor_type = "t" if name[:1].isupper() else "v"
    anchor = quote(name.replace("'", "-39-"), safe="-")
    return (
        f"https://hackage-content.haskell.org/package/{quote(package, safe='.-')}/docs/"
        f"{module_path}.html#{anchor_type}:{anchor}"
    )


def main(paths):
    source_files = defaultdict(set)
    frequencies = Counter()
    annotations = {}
    for path in paths:
        with open(path, encoding="utf-8") as source:
            records = json.load(source)
        source = path.rsplit("/", 1)[-1].removesuffix(".annot.json") + ".hs"
        for _line, _start, _end, text in records:
            annotation = parse_annotation(text)
            if annotation is None:
                continue
            name, signature, module, body = annotation
            frequencies[name] += 1
            annotations.setdefault(name, (signature, module, body))
            source_files[name].add(source)

    ordered = sorted(annotations, key=lambda name: (-frequencies[name], name))
    source_tags = load_source_tags(annotations)
    output = [
        "<!doctype html>",
        '<html lang="en">',
        '<head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">',
        '<title>Haskell annotations</title>',
        '<link rel="stylesheet" href="style.css">',
        '</head><body class="annotations">',
        "<h1>Haskell annotations</h1>",
        "<nav aria-label=\"Annotations by usage frequency\"><ol>",
    ]
    for name in ordered:
        href = escape(quote(name, safe=""), quote=True)
        output.append(
            f'<li><a href="#{href}"><code>{escape(name)}</code></a> '
            f'<span class="uses">({frequencies[name]} uses)</span></li>'
        )
    output.extend(["</ol></nav>", "<main>"])

    for name in ordered:
        signature, module, body = annotations[name]
        output.append('<section class="entry">')
        output.append(
            f'<h2 class="signature-heading"><code class="signature" id="{escape(name, quote=True)}">'
            f"{escape(signature)}</code>"
            f'<a class="signature signature-anchor" href="#{escape(name, quote=True)}">#</a></h2>'
        )
        files = "".join(
            f'<li><a href="{escape(quote(source[:-3] + ".html", safe="/"), quote=True)}">'
            f'{escape(source)}</a></li>'
            for source in sorted(source_files[name])
        )
        output.append(f'<ul class="used-in">Used in {files}</ul>')
        if module:
            url = defined_in_url(name, module, source_tags)
            linked_module = (
                f'<a href="{escape(url, quote=True)}"><code>{escape(module)}</code></a>'
                if url else f'<code>{escape(module)}</code>'
            )
            output.append(f'<p class="defined-in">, defined in {linked_module}</p>')
        output.append(f'<div class="documentation">{render_rst(body)}</div>')
        output.append("</section>")

    output.extend(["</main>", "</body></html>"])
    print("\n".join(output))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("usage: annot_html.py FILE.annot.json ...")
    main(sys.argv[1:])
