SRCS  = $(wildcard *.pod)
HTMLS = $(SRCS:.pod=.html)
PDFS  = $(SRCS:.pod=.pdf)
PSS   = $(SRCS:.pod=.ps)

all: ${HTMLS} ${PDFS}

%.html: %.pod
	pom2 html $< > $@

%.pdf: %.pod
	pom2 latex $< > $*.tex
	latex $*.tex
	pdflatex $*.tex

%.ps: %.pod
	pom2 latex $< > $*.tex
	latex $*.tex
	latex $*.tex
	dvips $*.dvi -o

clean:
	rm -f *.html *.tex *.toc *.aux *.log *.dvi *.ps *.pdf
