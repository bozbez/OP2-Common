### Documentation
To build documentation make sure that the following packages are installed
```
texlive-science
texlive-latex-extra
python-pygments
latex-xcolor
```

Usually the following will install these:
```
sudo apt-get install texlive-science texlive-latex-extra latex-xcolor
sudo apt-get install python-pygments (or alternatively run: sudo easy_install Pygments)
```

Then run the shell script `build_docs.sh`:
```
./build_docs.sh
```

This will build the C/C++ user documentation, OP2 developer documentation, OP2 MPI (distributed memory) developer documentation and airfoil application developer guide.
