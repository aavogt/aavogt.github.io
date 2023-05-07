Jfun.so: Jfun.f90
	gfortran -O3 -fPIC -shared  -g Jfun.f90 libslatec.so -llapack -o Jfun.so
Jversion.html: Jversion.md
	pandoc -s Jversion.md --mathjax -o Jversion.html

