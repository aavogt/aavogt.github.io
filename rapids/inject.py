#!/usr/bin/env python3
"""stdin: HsColour -css -partial html; argv[1]: .annot.json; stdout: html with doc links and tooltips."""
import sys, json, html, re
from html.parser import HTMLParser
from urllib.parse import quote

SVG_LITERAL = re.compile(r'"((?:[^"\\]|\\.)+\.svg)"')

ann = {(l, c): (e - c, t) for l, c, e, t in json.load(open(sys.argv[1]))}

class Inject(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=False)
        self.out, self.line, self.col = [], 1, 1

    def advance(self, s):
        for ch in s:
            if ch == "\n": self.line += 1; self.col = 1
            else: self.col += 1

    def handle_starttag(self, tag, attrs): self.out.append(self.get_starttag_text())
    def handle_endtag(self, tag):          self.out.append(f"</{tag}>")
    def handle_entityref(self, name):      self.out.append(f"&{name};"); self.col += 1
    def handle_charref(self, name):        self.out.append(f"&#{name};"); self.col += 1

    def handle_data(self, data):
        hit = ann.get((self.line, self.col))
        if hit and "\n" not in data[:hit[0]]:
            n, text = hit
            tip = html.escape(text, quote=True).replace("\n", "&#10;")
            name, separator, _ = text.partition(" :: ")
            span = (f'<span class="hs-doc" title="{tip}">'
                    f'{html.escape(data[:n])}</span>')
            if separator:
                href = html.escape(quote(name, safe=""), quote=True)
                self.out.append(f'<a href="annot.html#{href}">{span}</a>'
                                f'{html.escape(data[n:])}')
            else:
                self.out.append(span + html.escape(data[n:]))
        else:
            svg = SVG_LITERAL.fullmatch(data)
            if svg:
                href = html.escape(quote(svg.group(1), safe="/"), quote=True)
                self.out.append(f'<a href="{href}">{html.escape(data, quote=False)}</a>')
            else:
                self.out.append(html.escape(data, quote=False))
        self.advance(data)

p = Inject(); p.feed(sys.stdin.read()); print("".join(p.out), end="")
