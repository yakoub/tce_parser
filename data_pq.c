#include <libpq-fe.h>

void main() {
  PGconn * tce_db = PQconnectdb("postgresql://etl@localhost/tcestats");
}
