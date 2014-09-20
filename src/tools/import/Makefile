CC=gcc
CXX=g++


OBJ_DIR=.libs

TARGET=$(OBJ_DIR)/libthorcomm.so

CFLAGS=
LIBS=
INCLUDES+=-I../../add-ons

CFLAGS+=-fPIC -DPIC -pthread -Werror 

OBJS=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(wildcard *.cpp))


$(shell  mkdir -p $(OBJ_DIR))

#rm -f  $@; ar -r  $@ $^
$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) -shared -o $@ $^ $(LIBS)

$(OBJS):$(OBJ_DIR)/%.o:%.cpp
	$(CXX) $(INCLUDES) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(OBJ_DIR)/*
