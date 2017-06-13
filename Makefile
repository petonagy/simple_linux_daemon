 # Author:          Peter Nagy
 #
 # File:            Makefile
 # Project:         Simple linux daemon
 # Popis:           Daemon listening on specific port
 # Date:            12.6.2017

FILE            = daemon
FILE_SOURCES    = daemon.c

FLAGS           = -g -std=gnu99 -Wall -Wextra -pedantic

LIBRARIES       = -lpcap

CC              = gcc

$(FILE): $(FILE_SOURCES)
	$(CC) $(FLAGS) $< -o $@

clean:
	rm -f $(FILE)