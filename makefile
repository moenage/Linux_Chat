all: my_lets-talk

my_lets-talk:
	gcc -Wall -o lets-talk list.c lets-talk.c

clean:
	$(RM) lets-talk