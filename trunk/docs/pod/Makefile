# This is intedend to be used with the Jona-derived POD::POM package v15j
# Other pod processors will not be able to deal with pod extensions.
# Required syntax is
#   $(POMBIN)/pom2 html $< > $@

GLEDRELEASE=	"gled-1.1.7-alpha"

#export INCLUDE_CSS=	true

prefix=		/usr/local
exec_prefix=	$(prefix)
mandir=		$(prefix)/man
man1dir=	$(mandir)/man1
man7dir=	$(mandir)/man7
htmldir=	$(prefix)/shared/docs/$(GLEDRELEASE)

POMPREFIX=	Pod-POM-0.15/
#PERL5LIB:=	$(POMPREFIX)lib/:$(PERL5LIB)
POMBIN=		$(POMPREFIX)bin/
POM2=		perl -I$(POMPREFIX)/lib $(POMBIN)pom2

HTMLTITLE= 	"Gled system documentation"
MANTITLE= 	"Gled system manual pages"
DOCROOT= 	"../"
HTMLDIR= 	gled_doc_html
HTMLROOT= 	$(DOCROOT)$(HTMLDIR)/
TXTROOT= 	$(DOCROOT)txt/
PODROOT= 	$(DOCROOT)pod/
MANROOT= 	$(DOCROOT)man/
MANSEC=		7

PODS	:= $(wildcard *.pod)
MANS	:= gled.1 $(filter-out gled.7,$(PPODS:.pod=.7))
HTMLS	:= $(PODS:.pod=.html)
PDFS	:= $(PODS:.pod=.pdf)

# all : 	html pdf man gleddocs.pdf gled_docs.html.tgz dist
all : 	html pdf gled_docs.html.tgz dist

man :	${MANS}

html :	${HTMLS}

pdf :	${PDFS}

test:
	echo $(PERL5LIB)

# Actual rules; do not edit bellow this line


%.html : %.pod gled_doc.css
	$(POM2) html  $< >  $@ 
# && $(HTMLPP) $@

%.man %.7 %.1 %.2 : %.pod 
#	$(POM2) man --mansec $(MANSEC) --manhead $(MANTITLE)  $< >  $@ 
	$(POM2) text $< >  $@ 

%.tex : %.pod
	$(POM2) latex  $< >  $@ 

%.dvi : %.tex
	latex $< $@

%.pdf : %.dvi
	dvipdf $< $@

%.ps : %.dvi
	dvips $< -o $@

#%.inc : %.pod
#	pod2latex -modify -sections '!SEE ALSO|!AUTHORS|!TODO|!TODOS' -out $@  $< && mv $@.tex $@
#	pod2latex $<  $@ && mv $*.tex $@

#%.pod : %.ppod
#	$(PPODPP) $< > $@

html-dist: html
	mkdir -p $(HTMLROOT)/; cp *.html $(HTMLROOT)/; cp -R img/ examples/ $(HTMLROOT)/

pod-dist:
	mkdir -p $(PODROOT)/; cp *.pod $(PODROOT)/

man-dist: man
	mkdir -p $(MANROOT)/; cp *.[1-9] $(MANROOT)/

txt-dist: pod INSTALL.pod
	pod2text -l -s INSTALL.pod > INSTALL; cp INSTALL $(DOCROOT)/

#dist: pod-dist html-dist man-dist txt-dist
dist: pod-dist html-dist

gleddocs.tex:
	cp src/gleddocs.tex .

#gled_doc.css: src/gled_doc.css 
#	cp src/gled_doc.css .

#XXX We need to fix, references, toc, index, man sections...
gled_docs.html.tgz : html-dist
	tar -c -v -z -C $(DOCROOT) -f gled_docs.html.tgz $(HTMLDIR)/

gleddocs.dvi : gleddocs.tex gled.inc gledtoc.inc gledintro.inc gledparadigms.inc gledguts.inc gledbaseclasses.inc gledcoding.inc gledbuilding.inc

clean :
	rm -vf *.[1-9] *.man *.cat *.inc *.tex *.dvi *.pdf *.html *.log  

distclean : clean
	rm -Rvf *.aux *.idx *.tmp *.toc *~ gled_docs.html.tgz

realclean : clean
	rm -rf $(PODROOT)/* $(HTMLROOT)/* $(MANROOT)/* $(DOCROOT)/INSTALL $(DOCROOT)/README 
