SHELL = /bin/bash
filename=report

pdf:
	pdflatex ${filename}.tex
	make clean

clean:
	rm -f ${filename}.{ps,log,aux,out,dvi,bbl,blg}
