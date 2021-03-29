PROG = smartmet-grid-test-config-creator

REQUIRES = gdal configpp

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

CFLAGS = -DUNIX -O0 -g $(FLAGS) -Wno-unknown-pragmas

LIBS += -lsmartmet-grid-files -lsmartmet-macgyver

all: $(PROG)

clean:
	rm -f $(PROG) *~

install:
	mkdir -p $(bindir)
	mkdir -p $(datadir)/smartmet/test/grid/engine
	mkdir -p $(datadir)/smartmet/test/grid/library
	mkdir -p $(datadir)/smartmet/test/grid/redis
	$(INSTALL_DATA) grid/engine/*.cfg grid/engine/*.conf grid/engine/*.lua grid/engine/*.csv \
		$(datadir)/smartmet/test/grid/engine/
	$(INSTALL_DATA) grid/library/*csv grid/library/grid-files.conf \
		$(datadir)/smartmet/test/grid/library/
	$(INSTALL_DATA) grid/redis/redis-server.rdb grid/redis/redis-template.conf \
		$(datadir)/smartmet/test/grid/redis/
	$(INSTALL_PROG) smartmet-grid-test-config-creator $(bindir)/

$(PROG) : % : %.cpp Makefile
	$(CXX) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)
