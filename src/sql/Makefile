TARGET=${THOR_HOME}/lib/libthorsql.so
include ${THOR_HOME}/src/makefile.in

LEMON=${THOR_HOME}/add-ons/lemon

# for the parse.c
INCLUDES+=-w

parse.c:parse.y
	$(LEMON) $<

LIBS+=

