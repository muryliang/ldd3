objs = a.o b.o 
deps = a.d b.d
exes = a b c d
ver := 21
kk = $(if $(ver:1=),@,yy)
$(warning kk is $(kk))
#VPATH = headers
vpath %.h headers
obj = $(wildcard *.o)
#$(warning $(objs))
squote := '
$(warning hahah)
escsq = $(subst $(squote),'\$(squote)',$1)
$(warning $(call escsq,ls))

all: a c d  
.PHONY : all
$(warning $(CURDIR)/Makefile)
a : $(objs) $(deps)
	@echo $(two-lines)
	gcc -o a $(objs)

$(objs) : b.h
$(filter %.o,$(objs) d c ) : %.o : %.c
	$(warning $^)
	$(warning $<)
	$(warning $(obj))
	$(warning "next")
	$(CC) -c  $< -o $@

$(deps) : %.d : %.c
	@set -e ; rm -f $@; \
	$(CC) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@

c d : c.o 
	gcc c.o -o $@

define two-lines
echo aa
echo $(objs)
endef

.PHONY : clean
clean:
	$(warning $(obj))
	-rm  -f $(exes)  $(objs)  $(deps) a.d.* b.d.* *.o

