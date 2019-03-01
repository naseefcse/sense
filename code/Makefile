include optimizations.mk
include definitions.mk
MAKE=make
HTMLDIR=/cs/cheng3/public.html/sense
VERSION=no-version
all:
	cd libraries/parseConfig/src && $(MAKE)
	cd libraries/visualizer/src && $(MAKE)
	cd compc++ && $(MAKE) CC='$(CC)' CCFLAGS='$(CCFLAGS)'
	cd examples/compc++  && $(MAKE) CC='$(CC)' CCFLAGS='$(CCFLAGS)'
	cd examples/cost  && $(MAKE) CC='$(CC)' CCFLAGS='$(CCFLAGS)'
	cd examples/sense  && $(MAKE) CC='$(CC)' CCFLAGS='$(CCFLAGS)'
clean:
	cd libraries/parseConfig/src && $(MAKE) -i clean
	cd libraries/visualizer/src && $(MAKE) -i clean
	cd compc++ && $(MAKE) -i clean
	cd examples/cost && $(MAKE) -i clean
	cd examples/compc++ && $(MAKE) -i clean
	cd examples/sense && $(MAKE) -i clean
	find . -type f -name *.h -exec chmod -x {} \;
	find . -type f -name *.cc -exec chmod -x {} \;
	find . -name *.bak -type f -exec rm -f {} \;
	rm -f bin/cxx*

test:
	cd examples/sense && $(MAKE) test 

# This has not been updated...
doc:
	c2html common/sense.h $(HTMLDIR)/sense.h.html
	c2html app/cbr.h $(HTMLDIR)/cbr.h.html
	c2html net/flooding.h $(HTMLDIR)/flooding.h.html
	c2html mac/null_mac.h $(HTMLDIR)/null_mac.h.html
	c2html examples/sense/sim_routing.cc $(HTMLDIR)/sim_routing.cc.html
	c2html examples/cost/mm1.cc $(HTMLDIR)/mm1.cc.html
	c2html examples/cost/mmm1.cc $(HTMLDIR)/mmm1.cc.html

# This has not been updated...
dist:
	mkdir release
	cd release && cvs export -D tomorrow -d sense-$(VERSION) sense-2.0
	cd release && tar czf sense-$(VERSION).tar.gz sense-$(VERSION)
	cd release && zip -r sense-$(VERSION).zip sense-$(VERSION)
	chmod -w release/sense-$(VERSION).tar.gz
	chmod -w release/sense-$(VERSION).zip
	mv release/sense-$(VERSION).tar.gz $(HTMLDIR)/download
	mv release/sense-$(VERSION).zip $(HTMLDIR)/download
	rm -rf release


