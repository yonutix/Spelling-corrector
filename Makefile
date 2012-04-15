build : tema.c
	gcc -Wall -o2 -o tema tema.c

run : tema
	./tema

clean : tema.c
	rm tema
