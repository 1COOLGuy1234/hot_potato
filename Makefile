CFLAGS=-std=gnu99 -pedantic -Wall -Werror -ggdb3
# CFLAGS=-std=gnu99
PROGS=ringmaster player
OBJS=$(patsubst %,%.o,$(PROGS)) potato.o
all: $(PROGS)
ringmaster: ringmaster.o potato.o
	gcc -o $@ $^
player: player.o potato.o
	gcc -o $@ $^
%.o: %.c
	gcc $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f *~ $(PROGS) $(OBJS)

potato.o: potato.h