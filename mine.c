#include "tce_parse.h"

void _main() {
  const char *line = "801:41 InitGame: \\voteFlags\\4352\\g_balancedteams\\1\\g_realism\\1\\g_starthonor\\0\\g_killmessage\\0\\g_leanmode\\1\\g_newbbox\\1\\g_covertopsChargeTime\\30000\\tce_version\\Beta 0.490b PUB 20-12-2006 (Hardballer)\\g_soldierChargeTime\\20000\\g_LTChargeTime\\40000\\g_engineerChargeTime\\30000\\g_medicChargeTime\\45000\\g_bluelimbotime\\30000\\g_redlimbotime\\30000\\gamename\\tcetest\\cm_optimizePatchPlanes\\0\\g_axismaxlives\\0\\g_alliedmaxlives\\0\\sv_official\\0\\g_voteFlags\\0\\g_minGameClients\\8\\g_needpass\\0\\sv_privateClients\\4\\mapname\\obj_fracture_v2\\protocol\\84\\g_gametype\\5\\version\\ET 2.60b linux-i386 May  8 2006\\g_maxlivesRespawnPenalty\\0\\g_maxGameClients\\0\\g_heavyWeaponRestriction\\100\\g_antilag\\1\\g_maxlives\\0\\g_friendlyFire\\0\\sv_floodProtect\\1\\sv_maxPing\\0\\sv_minPing\\0\\sv_dlRate\\100\\sv_maxRate\\25000\\sv_minRate\\0\\sv_hostname\\T.H.E Server obj\\timelimit\\3\\sv_maxclients\\20";

  tce_parse(line);
}

void main() {
  const char *line = "817:16 ClientUserinfoChanged: 4 n\\^4E^7t^1s^9|^4HAL ^79000\\t\\3\\c\\0\\r\\0\\m\\0000000\\s\\0000000\\dn\\\\dr\\0\\w\\0\\lw\\0\\sw\\0\\mu\\0\\ref\\0";
  const char *line2 = "817:16 ClientUserinfoChanged: 4 n\\uuuuuuuuuu\\t\\37\\c\\0\\r\\0\\m\\0000000\\s\\0000000\\dn\\\\dr\\0\\w\\0\\lw\\0\\sw\\0\\mu\\0\\ref\\0";
  const char *line3 = "817:16 ClientUserinfoChanged: 2 n\\sdfdsuuuu\\t\\37\\c\\0\\r\\0\\m\\0000000\\s\\0000000\\dn\\\\dr\\0\\w\\0\\lw\\0\\sw\\0\\mu\\0\\ref\\0";

  tce_parse(line);
  tce_parse(line3);
  tce_parse(line2);
}
