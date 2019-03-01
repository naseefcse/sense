include ../../optimizations.mk
include ../../definitions.mk

TARGETS= hello-incl hello-system hello-funct print-int print-array

CXX_FILES=$(TARGETS:%=%.cxx)

all:	$(TARGETS)

$(CXX_FILES): %.cxx : %.cc
	$(COMPCXX) $<

$(TARGETS): %: %.cxx
	$(CC) $(CCFLAGS) -o $@ $< -lstdc++

clean:
	rm -f $(TARGETS) $(CXX_FILES) compcxx_*.h


