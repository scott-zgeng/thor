CC=gcc

LIBEV=libev-4.15
LIBLFDS=liblfds611

all:lemon ev.o $(LIBLFDS).a

lemon:lemon.c 
	$(CC) -O $< -o $@ 
	

ev.o:
	gzip -dc $(LIBEV).tar.gz | tar -xvf -
	cd $(LIBEV); ./configure; make;
	cp $(LIBEV)/.libs/ev.o ./
	cp $(LIBEV)/ev.h ./
	

$(LIBLFDS).a:
	unzip liblfds*.zip
	cd liblfds*/liblfds*; make -f makefile.linux arrel
	cp liblfds*/liblfds*/bin/$(LIBLFDS).a ./
	cp liblfds*/liblfds*/inc/* ./
	


.PHONY:
clean:
	rm -rf lemon ev.o $(LIBEV) liblfds6.1.1 $(LIBLFDS).a
