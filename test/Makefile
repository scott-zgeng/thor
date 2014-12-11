CXX=g++

CFLAGS+=-fPIC -DPIC -pthread -Werror -Wall  -g 
CFLAGS+=-O2
CFLAGS+=-msse4.2
CFLAGS+=-mavx

LINK_OPTION=-ldl

INCLUDES+=-I${THOR_HOME}/add-ons/gtest-1.7.0/include

INCLUDES+=-I${THOR_HOME}/src/common
INCLUDES+=-I${THOR_HOME}/src/kernel 
INCLUDES+=-I${THOR_HOME}/src/sql


LIBS+=-L${THOR_HOME}/lib -lthorcommon -lthorkernel -lthorsql 
LIBS+=-L${THOR_HOME}/add-ons/ -lgtest

TEST_BINS=$(patsubst %.h, %, $(wildcard test_*.h))

all:$(TEST_BINS)


$(TEST_BINS):%:%.h
	bash gen_test_file.sh $@	
	$(CXX) $(LINK_OPTION)  $(CFLAGS)  $(INCLUDES) -o $@ $@.cpp  $(LIBS)

.PHONY:
clean:
	rm -rf $(TEST_BINS) *.o *.cpp
	
test: $(TEST_BINS)
	@for test_case in $(TEST_BINS); do \
		./$$test_case; \
	done
