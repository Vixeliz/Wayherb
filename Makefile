CFLAGS = -DEGL_NO_X11 -DWLR_USE_UNSTABLE -Wall -g

INCLUDES = -Iinclude -Ilib -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/include/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng16

LIBS = -lEGL -lGLESv2 -lcairo -lwayland-cursor -lwayland-client -lwlroots -lwayland-egl

SRCS = src/util.c src/wayherb.c src/wayland.c lib/wlr-layer-shell-unstable-v1.c lib/xdg-shell.c

OBJS = $(SRCS:%.c=bin/%.o)

MAIN = wayherb

all: wayland $(MAIN)
	@echo Wayherb has succansfully compiled

wayland:
	wayland-scanner client-header lib/wlr-layer-shell-unstable-v1.xml lib/wlr-layer-shell-unstable-v1.h
	wayland-scanner private-code lib/wlr-layer-shell-unstable-v1.xml lib/wlr-layer-shell-unstable-v1.c

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o bin/$(MAIN) $(OBJS) $(CFLAGS) $(LIBS)

$(OBJS): wayland

bin/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS) *~ bin/wayherb
	rm -rf lib/wlr-layer-shell-unstable-v1.c
	rm -rf lib/wlr-layer-shell-unstable-v1.h

depend: $(SRCS)
	makedepend $(INCLUDES) $^

.PHONY: bin/%.o wayland depend clean

#END
