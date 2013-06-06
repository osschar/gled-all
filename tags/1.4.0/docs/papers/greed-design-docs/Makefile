SRCS  = $(wildcard *.pod)
HTMLS = $(SRCS:.pod=.html)
PDFS  = $(SRCS:.pod=.pdf)
PSS   = $(SRCS:.pod=.ps)

WEBHOST := www.gled.org
WEBDIR  := /u/www/docs/greed-design-docs

all: html pdf

html: ${HTMLS}

pdf: ${PDFS}

%.html: %.pod
	pod3 html $< > $@

%.pdf: %.pod
	pod3 latex $< > $*.tex
	latex $*.tex
	pdflatex $*.tex

%.ps: %.pod
	pod3 latex $< > $*.tex
	latex $*.tex
	latex $*.tex
	dvips $*.dvi -o

clean:
	rm -f *.html *.tex *.toc *.aux *.log *.dvi *.ps *.pdf

publish:
	scp *.html ${WEBHOST}:${WEBDIR}
