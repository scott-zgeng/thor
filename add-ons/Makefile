CC=gcc

all:lemon ev.o

lemon:lemon.c 
	$(CC) -O $< -o $@ 
	
LIBEV=libev-4.15
ev.o:
	gzip -dc $(LIBEV).tar.gz | tar -xvf -
	cd $(LIBEV); ./configure; make;
	cp $(LIBEV)/.libs/ev.o ./
	cp $(LIBEV)/ev.h ./

.PHONY:
clean:
	rm -rf lemon ev.o $(LIBEV)
