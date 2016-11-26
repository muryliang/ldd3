objs = a.o b.o 
deps = a.d b.d
exes = a b c d
#VPATH = headers
vpath %.h headers
obj = $(wildcard *.o)
#$(warning $(objs))
all: a c d  
.PHONY : all

a : $(objs) $(deps)
	gcc -o a $(objs)

$(objs) : b.h
$(filter %.o,$(objs) d c ) : %.o : %.c
	$(warning $^)
	$(warning $<)
	$(warning $(wildcard $^))
	$(warning "next")
	$(CC) -c  $< -o $@

$(deps) : %.d : %.c
	@set -e ; rm -f $@; \
	$(CC) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@

c d : c.o 
	gcc c.o -o $@

.PHONY : clean
clean:
#	$(warning $(obj))
	-rm  -f $(exes)  $(objs)  $(deps) a.d.* b.d.* *.o

