all: my_lets-talk

my_cshell:
	gcc -Wall -o lets-talk lets-talk.c

clean:
	$(RM) my_lets-talk