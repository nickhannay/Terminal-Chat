all: lets-talk

lets-talk: lets-talk.o sender.o receiver.o
	gcc -g -W -pthread lets-talk.o sender.o receiver.o list.c -o lets-talk

lets-talk.o: lets-talk.c
	gcc -c lets-talk.c

sender.o: sender.c
	gcc -c sender.c

receiver.o: receiver.c
	gcc -c receiver.c

clean:
	rm *.o lets-talk