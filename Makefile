LDFLAGS =
OBJDIR=obj

CFLAGS =
CPPFLAGS = -DEFTI_SW=1 -DEFTI_HW=0
SRCDIRS  = . datasets
INCLUDE = -I.
SRCS    := $(shell find $(SRCDIR) -name '*.c')
OBJS    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

release: CFLAGS += -Ofast
release: efti

debug:   CFLAGS += -g3
debug: efti

efti: buildrepo $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o efti

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)

buildrepo:
	@$(call make-repo)

define make-repo
	for dir in $(SRCDIRS); \
	do \
		mkdir -p $(OBJDIR)/$$dir; \
	done
endef
