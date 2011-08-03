all: sum.pdf

sum.pdf: sum.tex my-bibliography-file.bib
	pdflatex sum.tex
	bibtex sum
	pdflatex sum.tex
	pdflatex sum.tex
	open sum.pdf

clean:
	rm -rf *~ *.aux *.dvi *.log *.bbl *.blg
