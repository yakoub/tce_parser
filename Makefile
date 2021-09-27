myinc != mariadb_config --include
mylib != mariadb_config --libs
debug = -g 
objects = bin/watch.o bin/sync.o bin/tce_parse.o bin/data.o bin/config.o

all: tce_watch tce_reset

tce_watch: ${objects}
	gcc ${debug} ${objects} ${mylib} -o tce_watch 
tce_reset: sync_reset.c bin/config.o
	gcc bin/config.o sync_reset.c -o tce_reset

bin/watch.o: watch.c sync.h data.h debug.h
	gcc ${debug} -c watch.c -o bin/watch.o
bin/sync.o: sync.c sync.h tce_parse.h debug.h
	gcc ${debug} -c sync.c -o bin/sync.o
bin/tce_parse.o: tce_parse.c tce_parse.h data.h debug.h
	gcc ${debug} -c tce_parse.c -o bin/tce_parse.o
bin/data.o: data.c data.h debug.h
	gcc ${debug} -c ${myinc} data.c -o bin/data.o
bin/config.o: data.h conf.c
	gcc ${debug} -c conf.c -o bin/config.o
clean:
	rm bin/* tce_watch tce_reset
pack:
	tar -cf pack.tar *.h watch.c sync.c sync_reset.c 
	tar -rf pack.tar tce_parse.c data.c conf.tpl.c
