#ifndef SQLDEFS_H
#define SQLDEFS_H

#include "columnids.h"

// SQL table defs and sqlite defaults
#define SQLITE_DB_BASENAME "ZillowBrowser.sqlite"  // SQLITE filename

/* Database version history
 * Version  Notes
 *  1       First released version with version table (zbr_dbver)
 */

// Database version
#define SQLITE_DATABASE_VERSION 1
#define SQLITE_TABLEDEF_DBVER   "\
CREATE TABLE IF NOT EXISTS zbr_dbver (\
    version INTEGER PRIMARY KEY\
)\
"

// Property entry.
#define SQLITE_TABLEDEF_PROPERTY    "\
CREATE TABLE IF NOT EXISTS zbr_property (\
    id INTEGER PRIMARY KEY AUTOINCREMENT,\
    zpid INTEGER NOT NULL,\
    when_checked INTEGER NOT NULL,\
    detail_url TEXT NOT NULL,\
    lot_size TEXT,\
    year_built INTEGER,\
    price INTEGER,\
    bedrooms TEXT,\
    baths TEXT,\
    sqft INTEGER,\
    zestimate INTEGER,\
    schools TEXT,\
    schools_avg REAL,\
    hoa_fee TEXT,\
    address TEXT,\
    city TEXT,\
    county TEXT,\
    zip TEXT,\
    state TEXT,\
    latitude INTEGER,\
    longitude INTEGER,\
    user1 TEXT,\
    user2 TEXT,\
    user3 TEXT,\
    user4 TEXT,\
    user5 TEXT,\
    user6 TEXT,\
    user7 TEXT,\
    user8 TEXT,\
    user9 TEXT\
)\
"

// User column definitions
#define SQLITE_TABLEDEF_USERCOLS    "\
CREATE TABLE IF NOT EXISTS zbr_usercols (\
    name TEXT NOT NULL,\
    user_name TEXT,\
    collation INTEGER NOT NULL,\
    PRIMARY KEY (name)\
)\
"

// All table defs to assert
#define SQLITE_TABLEDEFS_ALL    SQLITE_TABLEDEF_DBVER, SQLITE_TABLEDEF_PROPERTY, SQLITE_TABLEDEF_USERCOLS

#endif // SQLDEFS_H
