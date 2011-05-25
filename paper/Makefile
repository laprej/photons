all: sum.pdf

sum.pdf: sum.tex my-bibliography-file.bib
	latex sum.tex
	bibtex sum
	latex sum.tex
	latex sum.tex
	dvipdf sum.dvi
	open sum.pdf

clean:
	rm -rf *~ *.aux *.dvi *.log
