# tce_parser
tce game log parser

using unix inotify api to watch log files updates, parsing the scores and writing to database .  

build by copying conf.tpl.c to conf.c and fill the in the configurations .  
install using executable to systemd using provided tce_watch.service file .  
