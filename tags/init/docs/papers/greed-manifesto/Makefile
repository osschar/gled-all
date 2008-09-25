clean:
	@rm -f *.aux *.log *.toc *.dvi *.ps *.pdf

%:
	latex $@.tex
	latex $@.tex
	dvips $@.dvi -o
	pdflatex $@.tex
