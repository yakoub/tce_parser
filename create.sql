drop table if exists `game_player`;
drop table if exists `game_match`;

create table `game_match` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `created` datetime not null default CURRENT_TIMESTAMP,
  `mapname` varchar(64),
  `hostname` varchar(64),
  `team_red` smallint,
  `team_blue` smallint,
  `gametype` smallint,

  primary key(`id`)
);

create table `game_player` (
  `match_id` bigint(20) not null,
  `idx` tinyint not null,
  `team` tinyint,
  `name` varchar(64),
  `ping` smallint,
  `score` smallint,
  `kills` smallint,
  `deaths` smallint,
  `headshots` smallint,
  `damage_given` smallint,
  `damage_recieved` smallint,

  primary key (`match_id`, `idx`),
  foreign key (`match_id`) references game_match (`id`) on delete cascade
);

create or replace view top_games as 
select count(*),id from game_match gm 
inner join game_player gp on gm.id=gp.match_id group by gm.id 
order by count(*) desc limit 10;

