#Versione test - non uso ottimizzazioni del compilatore


CFLAGS =

TARGET = test


CORE_SOURCES = core/core.c\
		core/calqueue.c\
		core/main.c\
		core/time_util.c\
		core/application.c
			

all:
	gcc -Wall -o $(TARGET) $(CORE_SOURCES) $(CFLAGS) -mrtm -pthread 