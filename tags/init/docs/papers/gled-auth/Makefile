TEXS := $(wildcard *.tex)
PSS  := $(TEXS:%.tex=%.ps)
PDFS := $(TEXS:%.tex=%.pdf)

%.dvi: %.tex
	latex $<
	latex $<

%.ps: %.dvi
	dvips $< -o

%.pdf: %.tex
	pdflatex $<

all: ${PSS} ${PDFS}

clean:
	@rm -f *.aux *.log *.dvi *.ps *.pdf
