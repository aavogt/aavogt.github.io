HS     := $(wildcard *.hs)
ANNOTS := $(HS:.hs=.annot)
ANNOTS_JSON := $(HS:.hs=.annot.json)
HTMLS := $(HS:.hs=.html)
GLBS := $(HS:.hs=.glb)
EXPL := $(addprefix explicit_imports/,$(HS))

index.html: index.html.proto $(HTMLS) $(GLBS) annot.html
	{ cat index.html.proto; \
	  echo '<nav><ul><li><a href="annot.html">Annotations</a></li>'; \
	  for f in $(HS); do b=$${f%.hs}; \
	    echo "<li><a href=\"#$$b\">$$f</a></li>"; done; \
	  echo '</ul></nav>'; \
	  for f in $(HS); do b=$${f%.hs}; \
	    echo "<section id=\"$$b\"><h1>$$f</h1>"; \
	    cat $$b.html; \
			echo "<model-viewer src=\"$$b.glb\" camera-controls interaction-prompt=none camera-orbit=\"auto auto 25%\"></model-viewer>"; \
	    echo '</section>'; done; \
	  echo '</body></html>'; \
	} > $@

.PHONY: serve clean allclean
serve: index.html annot.html
	python3 -m http.server 8080 --bind 127.0.0.1

$(ANNOTS) $(ANNOTS_JSON) &: $(HS) $(EXPL) annot.py
	python3 annot.py

annot.html: $(HS:.hs=.annot.json) annot_html.py Makefile
	python3 annot_html.py $(HS:.hs=.annot.json) > $@

%.glb: %.step
	python3 -c "import cascadio; cascadio.step_to_glb('$<', '$@')"

%.step: %.hs
	runghc $<

explicit_imports/%.hs: %.hs
	python3 explicit.py $< $@

.ONESHELL:

%.html: %.hs %.annot inject.py Makefile
	f=`mktemp`
	HsColour -css -partial -o$$f $<
	python3 inject.py $*.annot.json < $$f > $@
	rm $$f # or trap

clean:
	rm $(EXPL) $(ANNOTS) $(ANNOTS_JSON) $(HS:.hs=.step) $(HTMLS) annot.html

allclean:
	rm $(GLBS) index.html
