SUBDIRS=daemon import

all:
	@for dir in $(SUBDIRS); do \
		make -C $$dir; \
	done	

clean:
	@for dir in $(SUBDIRS); do \
		make -C $$dir $@; \
	done	
