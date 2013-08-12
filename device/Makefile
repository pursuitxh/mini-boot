SUBDIRS = serial misc net

SUB_BUILT_IN_OBJS := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: $(SUBDIRS) $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(SUB_BUILT_IN_OBJS)
	$(LD) $(LDFLAGS) -r $^ -o $@

$(SUBDIRS):
	@make -C $@ all

clean:
	@for dir in $(SUBDIRS); do \
	    make -C $$dir clean; \
	done
	@rm -vf *.o

.PHONY: $(SUBDIRS)

