run: watch.c sync.o tce_parse.o data.o
	gcc -g watch.c sync.o tce_parse.o data.o -o tce_watch
sync.o: sync.c sync.h tce_parse.h
	gcc -g -c sync.c
tce_parse.o: tce_parse.c tce_parse.h data.h
	gcc -g -c tce_parse.c
data.o: data.c data.h
	gcc -g -c data.c
clean:
	rm *.o tce_watch
