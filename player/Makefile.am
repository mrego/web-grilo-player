INCLUDES =					\
	-I$(top_srcdir)				\
	$(WARN_CFLAGS)				\
	$(PLAYER_CFLAGS)			\
        -DPLAYER_DIR=\""$(datadir)/player/"\"

noinst_PROGRAMS = web-grilo-player-example

web_grilo_player_example_SOURCES =	\
	web-grilo-player-example.c

web_grilo_player_example_LDFLAGS =	\
	$(PLAYER_LIBS)
