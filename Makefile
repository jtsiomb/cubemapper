# -- options --
PREFIX = /usr/local
opt = -O3
dbg = -g
# -------------

src = $(wildcard src/*.cc)
obj = $(src:.cc=.o)
dep = $(obj:.o=.d)
bin = cubemapper

CXXFLAGS = -pedantic -Wall $(opt) $(dbg) $(inc)
LDFLAGS = $(libs) $(libgl_$(sys)) -lm

sys = $(shell uname -s)
libgl_Linux = -lGL -lGLU -lglut -lGLEW
libgl_Darwin = -framework OpenGL -framework GLUT -lGLEW

libs = -limago -lgmath

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
