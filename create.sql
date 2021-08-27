drop table if exists `game_player`;
drop table if exists `game_match`;

create table `game_match` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `created` datetime not null default CURRENT_TIMESTAMP,
  `mapname` varchar(32),
  `hostname` varchar(32),
  `team_red` smallint,
  `team_blue` smallint,

  primary key(`id`)
);

create table `game_player` (
  `match_id` bigint(20) not null,
  `idx` tinyint not null,
  `team` tinyint,
  `name` varchar(32),
  `ping` smallint,
  `score` smallint,
  `kills` smallint,
  `deaths` smallint,
  `headshots` smallint,

  primary key (`match_id`, `idx`),
  foreign key (`match_id`) references game_match (`id`) on delete cascade
);
