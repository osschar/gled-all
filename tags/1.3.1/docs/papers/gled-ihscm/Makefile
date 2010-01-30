clean:
	@rm -f *.aux *.log *.dvi *.ps *.pdf

%:
	latex $@.tex
	latex $@.tex
	dvips $@.dvi -o
	pdflatex $@.tex
