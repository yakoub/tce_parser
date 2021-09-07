myinc != mariadb_config --include
mylib != mariadb_config --libs
debug = -g
objects = bin/sync.o bin/tce_parse.o bin/data.o bin/config.o
run: watch.c sync tce_parse data config
	gcc ${debug} watch.c ${objects} ${mylib} -o tce_watch 
sync: sync.c sync.h tce_parse.h debug.h
	gcc ${debug} -c sync.c -o bin/sync.o
tce_parse: tce_parse.c tce_parse.h data.h debug.h
	gcc ${debug} -c tce_parse.c -o bin/tce_parse.o
data: data.c data.h
	gcc ${debug} -c ${myinc} data.c -o bin/data.o
config: data.h
	gcc ${debug} -c conf.c -o bin/config.o
clean:
	rm bin/*
