HS     := $(wildcard *.hs)
ANNOTS := $(HS:.hs=.annot)
ANNOTS_JSON := $(HS:.hs=.annot.json)
HTMLS := $(HS:.hs=.html)
HTML1S := $(HS:.hs=.html1)
GLBS := $(HS:.hs=.glb)
PNGS := $(HS:.hs=.png)
.SECONDARY: $(HS:.hs=.glb1)
EXPL := $(addprefix explicit_imports/,$(HS))

index.html: $(HTMLS) $(PNGS) annot.html Makefile
	{ cat index.html.proto; \
	  echo '<ul><li><a href="annot.html">Annotations</a></li>'; \
	  for f in $(HS); do b=$${f%.hs}; \
	    echo "<li><a href=\"$$b.html\"><img src=$$b.png></img>$$f</a></li>"; done; \
	  echo '</ul></body></html>'; \
	} > $@

annot.html: $(HS:.hs=.annot.json) annot_html.py Makefile
	python3 annot_html.py $(HS:.hs=.annot.json) > $@

%.html: %.html1 index.html.proto
	{ cat index.html.proto; \
	  for f in $<; do b=$${f%.html1}; \
	    echo "<h1>$$f</h1>"; \
	    cat $$b.html1; \
			echo "<model-viewer src=\"$$b.glb\" camera-controls interaction-prompt=none camera-orbit=\"auto auto 25%\"></model-viewer>"; \
	    echo '</section>'; done; \
	  echo '</body></html>'; \
	} > $@

onepage.html: onepage.html.proto index.html.proto $(HTML1S) $(GLBS) annot.html
	{ cat index.html.proto; \
	  echo '<nav><ul><li><a href="annot.html">Annotations</a></li>'; \
	  for f in $(HS); do b=$${f%.hs}; \
	    echo "<li><a href=\"#$$b\">$$f</a></li>"; done; \
	  echo '</ul></nav>'; \
	  for f in $(HS); do b=$${f%.hs}; \
	    echo "<section id=\"$$b\"><h1>$$f</h1>"; \
	    cat $$b.html1; \
			echo "<model-viewer src=\"$$b.glb\" camera-controls interaction-prompt=none camera-orbit=\"auto auto 25%\"></model-viewer>"; \
	    echo '</section>'; done; \
	  echo '</body></html>'; \
	} > $@


commit: index.html onepage.html $(HTMLS) $(GLBS) annot.html
	git add $< $(HS) *.svg
	git commit -m 'rapids_examples'

.PHONY: serve clean allclean
serve: index.html annot.html
	python3 -m http.server 8080 --bind 127.0.0.1

$(ANNOTS) $(ANNOTS_JSON) &: $(HS) $(EXPL) annot.py
	python3 annot.py


%.glb1: %.step
	python3 -c "import cascadio; cascadio.step_to_glb('$<', '$@')"

%.glb: %.glb1 add_specular.py
	cp $< $@
	python3 add_specular.py $@

%.png: %.glb
	f3d $< --output=$@
	mogrify -trim +repage -scale 20% $@

%.step: %.hs
	runghc $<

explicit_imports/%.hs: %.hs
	python3 explicit.py $< $@

.ONESHELL:

%.html1: %.hs %.annot annot.html inject.py Makefile
	f=`mktemp`
	HsColour -css -partial -o$$f $<
	python3 inject.py $*.annot.json < $$f > $@
	rm $$f # or trap

clean:
	rm $(EXPL) $(ANNOTS) $(ANNOTS_JSON) $(HS:.hs=.glb1) $(HS:.hs=.step) $(HTML1S)

allclean:
	rm $(GLBS) index.html annot.html $(HTMLS) $(PNGS)
