clean:
	@rm -f *.aux *.log *.dvi *.ps *.pdf

%:
	latex $@.tex
#	latex $@.tex
	dvips  -T 29.7cm,21cm $@.dvi -o
#	pdflatex $@.tex
