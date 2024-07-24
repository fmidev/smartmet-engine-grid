SUBNAME = grid
SPEC = smartmet-engine-$(SUBNAME)
INCDIR = smartmet/engines/$(SUBNAME)

REQUIRES = libpq configpp gdal

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

# Enabling / disabling CORBA usage.

CORBA = enabled

DEFINES = -DUNIX -D_REENTRANT

LIBS += $(PREFIX_LDFLAGS) \
	$(REQUIRED_LIBS) \
	-lsmartmet-macgyver \
	-lsmartmet-gis \
	-lsmartmet-spine \
	-lboost_regex \
	-lboost_thread \
	-lboost_iostreams \
	-lboost_system \
	-lbz2 -lz

ifeq ($(CORBA), disabled)
  CORBA_FLAGS = -DCORBA_DISABLED
else
  CORBA_INCLUDE = -isystem /usr/include/smartmet/grid-content/contentServer/corba/stubs \
                  -isystem /usr/include/smartmet/grid-content/dataServer/corba/stubs \
                  -isystem /usr/include/smartmet/grid-content/queryServer/corba/stubs
  CORBA_LIBS = -lomniORB4 -lomnithread
endif

INCLUDES += \
	-I$(includedir)/smartmet \
	$(CORBA_INCLUDE)

LIBS += -L$(libdir) \
	$(REQUIRED_LIBS) \
	$(CORBA_LIBS) \
	-lsmartmet-spine \
	-lsmartmet-grid-files \
	-lsmartmet-grid-content \
	-lboost_thread \
	-lboost_system \
	-lpthread

# What to install

LIBFILE = $(SUBNAME).so

# Compilation directories

vpath %.cpp $(SUBNAME)
vpath %.h $(SUBNAME)

# The files to be compiled

SRCS = $(wildcard $(SUBNAME)/*.cpp)
HDRS = $(wildcard $(SUBNAME)/*.h)
OBJS = $(patsubst %.cpp, obj/%.o, $(notdir $(SRCS)))

INCLUDES := -Iinclude $(INCLUDES)

.PHONY: test rpm

# The rules

all: objdir $(LIBFILE)
	$(MAKE) -C testdata $@

debug: all
release: all
profile: all

configtest:
	@if [ -x "$$(command -v cfgvalidate)" ]; then cfgvalidate -v test/cnf/grid-engine.conf; fi

$(LIBFILE): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -rdynamic -o $(LIBFILE) $(OBJS) $(LIBS)
	@echo Checking $(LIBFILE) for unresolved references
	@if ldd -r $(LIBFILE) 2>&1 | c++filt | grep ^undefined\ symbol |\
                        grep -Pv ':\ __(?:(?:a|t|ub)san_|sanitizer_)'; \
		then rm -v $(LIBFILE); \
		exit 1; \
	fi

clean:
	rm -f $(LIBFILE) obj/* *~ $(SUBNAME)/*~
	$(MAKE) -C testdata $@

clean-install:
	rm -rf $(includedir)/$(INCDIR)/*
	rm -f $(enginedir)/$(LIBFILE)

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp test/*.cpp

install:
	@mkdir -p $(includedir)/$(INCDIR)
	@list='$(HDRS)'; \
	for hdr in $$list; do \
	  HDR=$$(basename $$hdr); \
	  echo $(INSTALL_DATA) $$hdr $(includedir)/$(INCDIR)/$$HDR; \
	  $(INSTALL_DATA) $$hdr $(includedir)/$(INCDIR)/$$HDR; \
	done
	@mkdir -p $(enginedir)
	$(INSTALL_PROG) $(LIBFILE) $(enginedir)/$(LIBFILE)
	$(MAKE) -C testdata $@


test:
	if [ -d test/Makefile ] ; then $(MAKE) -C test $@; else true; fi

objdir:
	@mkdir -p $(objdir)

rpm: clean $(SPEC).spec
	rm -f $(SPEC).tar.gz # Clean a possible leftover from previous attempt
	tar -czvf $(SPEC).tar.gz --exclude-vcs --transform "s,^,$(SPEC)/," *
	rpmbuild -tb $(SPEC).tar.gz
	rm -f $(SPEC).tar.gz

.SUFFIXES: $(SUFFIXES) .cpp

obj/%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst obj/%.o, obj/%.d, $@) -MT $@ -o $@ $<

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
