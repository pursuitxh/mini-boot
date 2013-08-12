objs := div64.o lib1funcs.o ctype.o muldi3.o printf.o string.o vsprintf.o

all: $(BUILT_IN_OBJ)
	
$(BUILT_IN_OBJ): $(objs)
	$(LD) -r $(LDFLAGS) -o $@ $^
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o		
	
