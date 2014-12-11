CC=gcc

LIBEV=libev-4.15
LIBLFDS=liblfds611
LIBGTEST=gtest-1.7.0

all:lemon ev.o $(LIBLFDS).a libgtest.a

lemon:lemon.c 
	$(CC) -O $< -o $@ 
	

ev.o:
	gzip -dc $(LIBEV).tar.gz | tar -xvf -
	cd $(LIBEV); ./configure; make;
	cp $(LIBEV)/.libs/ev.o ./
	cp $(LIBEV)/ev.h ./
	

$(LIBLFDS).a:
	gzip -dc $(LIBLFDS).tar.gz | tar -xvf -	
	cd $(LIBLFDS); make arrel
	cp $(LIBLFDS)/bin/$(LIBLFDS).a ./
	cp $(LIBLFDS)/inc/* ./
	
libgtest.a:
	rm -rf $(LIBGTEST)
	unzip  $(LIBGTEST).zip
	cd $(LIBGTEST); ./configure; make
	cp $(LIBGTEST)/lib/.libs/libgtest.a ./

.PHONY:
clean:
	rm -rf lemon ev.o $(LIBEV) $(LIBLFDS) $(LIBLFDS).a $(LIBGTEST) libgtest.a 
