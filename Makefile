LDFLAGS =
OBJDIR = rel

CFLAGS =
CPPFLAGS = -DEFTI_SW=1 -DEFTI_HW=0
SRCDIRS  = src src/datasets
INCLUDE = -Isrc
SRCS    := $(shell find $(SRCDIRS) -maxdepth 1 -name '*.c')
OBJS    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

rel: CFLAGS += -Ofast
rel: app

dbg:   CFLAGS += -g3
dbg: app

app: $(OBJDIR)/efti

$(OBJDIR)/efti: buildrepo $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(OBJDIR)/efti

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
