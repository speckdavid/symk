
# $Id$

DOCS = \
	InterfaceDoc.pdf

FIGS = 

all:	$(FIGS) $(DOCS)

clean:  
	rm $(FIGS) $(DOCS)

%.eps: %.ps
	ps2eps $< $@

%.ps: %.dvi
	dvips -t letter -o $@ $<

%.dvi: %.tex
	latex $<

%.pdf: %.ps
	ps2pdf $< $@ 

