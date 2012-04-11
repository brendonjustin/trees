proposal.pdf: proposal.tex
	pdflatex proposal.tex
#	bibtex proposal.aux
	pdflatex proposal.tex
#	pdflatex proposal.tex

clean:
	rm *.aux *.log proposal.pdf
