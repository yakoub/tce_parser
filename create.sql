select 'drop game_player' as stage;
drop table if exists `game_player`;
select 'drop index' as stage;
drop table if exists `player_index`;
select 'drop game_match' as stage;
drop table if exists `game_match`;
select 'drop game_server' as stage;
drop table if exists `game_server`;


create table `game_server` (
  `id` int unsigned not null auto_increment,
  `hostname` varchar(64),
  `hostname_plain` varchar(64),
  `port` int unsigned,
  `ip` varchar(16),

  primary key(`id`)
);

create table `game_match` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `created` datetime not null default CURRENT_TIMESTAMP,
  `mapname` varchar(64),
  `server_id` int unsigned not null,
  `team_red` smallint,
  `team_blue` smallint,
  `gametype` smallint,

  foreign key (`server_id`) references game_server (`id`) on delete cascade,
  primary key(`id`)
);

create table `player_index` (
  `guid` varchar(33),
  `name` varchar(64),
  `name_plain` varchar(64),
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `bot` boolean not null default FALSE,
  primary key(`id`),
  unique(guid, name)
);

create table `game_player` (
  `match_id` int unsigned not null,
  `player_id` int unsigned not null,
  `idx` tinyint not null,
  `team` tinyint,
  `ping` smallint,
  `score` smallint,
  `kills` smallint,
  `deaths` smallint,
  `headshots` smallint,
  `damage_given` smallint,
  `damage_recieved` smallint,

  primary key (`match_id`, `idx`),
  foreign key (`match_id`) references game_match (`id`) on delete cascade,
  foreign key (`player_id`) references player_index (`id`) on delete cascade
);

create or replace view top_games as 
select count(*),id from game_match gm 
inner join game_player gp on gm.id=gp.match_id group by gm.id 
order by count(*) desc limit 10;

