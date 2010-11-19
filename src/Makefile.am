INCLUDES =					\
	-I$(top_srcdir)				\
	$(WARN_CFLAGS)				\
	$(PLAYER_CFLAGS)			\
        -DHTML_DIR=\""$(datadir)/data/html/"\"

noinst_PROGRAMS = wgp

wgp_SOURCES =		\
	wgp-main.c	\
	wgp-view.h	\
	wgp-view.c

wgp_LDFLAGS =	\
	$(PLAYER_LIBS)
