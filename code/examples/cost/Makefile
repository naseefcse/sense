include ../../optimizations.mk
include ../../definitions.mk
MAKE=

TARGETS= mm1 mm1-comp mm1-server mmm1
CXX_FILES=$(TARGETS:%=%.cxx)

all:	$(TARGETS)

$(CXX_FILES): %.cxx : %.cc
	$(COMPCXX) $<

$(TARGETS): %: %.cxx
	$(CC) $(CCFLAGS) -o $@ $< -lstdc++

clean:
	rm -f $(TARGETS) $(CXX_FILES) compcxx_*.h
