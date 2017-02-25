LDFLAGS =
OBJDIR = rel

CXXFLAGS = -DEFTI_SW=1 -DEFTI_HW=0
SRCDIRS  = src src/datasets
INCLUDE = -Isrc
SRCS    := $(shell find $(SRCDIRS) -maxdepth 1 -name '*.c')
OBJS    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPDIR := $(OBJDIR)/.d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

echo:
	@echo $(SRCS)

rel: CXXFLAGS += -DNDEBUG -Ofast -fPIC
rel: app

dbg:   CXXFLAGS += -g3
dbg: app

test-dbg:   CXXFLAGS += -g3
test-dbg: test

app: $(OBJDIR)/efti

$(OBJDIR)/efti: buildrepo $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(OBJDIR)/efti

$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(CXX) $(DEPFLAGS) $(INCLUDE) $(CXXFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

rel/test.o: src/test/test.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $(OBJDIR)/test.o src/test/test.cpp

test: buildrepo $(OBJS) rel/test.o
	$(CXX) $(CXXFLAGS) -o $(OBJDIR)/test rel/test.o $(filter-out rel/src/main.o, $(OBJS)) $(LDFLAGS)
	./rel/test

clean:
	$(RM) $(OBJS)

buildrepo:
	@$(call make-repo)

define make-repo
	for dir in $(SRCDIRS); \
	do \
		mkdir -p $(OBJDIR)/$$dir; \
		mkdir -p $(DEPDIR)/$$dir; \
	done
endef

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))
