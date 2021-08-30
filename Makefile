myinc != mariadb_config --include
mylib != mariadb_config --libs

run: watch.c sync.o tce_parse.o data.o
	gcc -g ${mylib} watch.c sync.o tce_parse.o data.o -o tce_watch
sync.o: sync.c sync.h tce_parse.h debug.h
	gcc -g -c sync.c
tce_parse.o: tce_parse.c tce_parse.h data.h debug.h
	gcc -g -c tce_parse.c
data.o: data.c data.h
	gcc -g -c ${myinc} data.c
clean:
	rm *.o tce_watch
