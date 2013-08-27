WEBHOST := www.gled.org
WEBDIR  := /u/www/docs/greed-manifesto

all: greed-manifesto

clean:
	@rm -f *.aux *.log *.toc *.dvi *.ps *.pdf

greed-manifesto:
	latex $@.tex
	latex $@.tex
	dvips $@.dvi -o
	pdflatex $@.tex

publish:
	scp *.ps *.pdf ${WEBHOST}:${WEBDIR}
