#ifndef SQLDEFS_H
#define SQLDEFS_H

#include "columnids.h"

// SQL table defs and sqlite defaults
#define SQLITE_DB_BASENAME "ZillowBrowser.sqlite"  // SQLITE filename

/* Database version history
 * Version  Notes
 *  1       First released version with version table (zbr_dbver)
 *  2       Added weighting table
 */

// Database version
#define SQLITE_DATABASE_VERSION 2
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

// Property table fields corresponding to columns used for array initialization
#define PROPERTY_TABLE_FIELDS "_nd_detail_url", "_nd_id", "lot_size", "year_built", "price", "bedrooms", "baths", "sqft", "zestimate", "schools", "hoa_fee", \
    "_nd_address", "_nd_city", "_nd_county", "_nd_zip", "_nd_state", "_nd_geo", "_c_distance", "user1", "user2", "user3", "user4", "user5", \
    "user6", "user7", "user8", "user9"

// User column definitions
#define SQLITE_TABLEDEF_USERCOLS    "\
CREATE TABLE IF NOT EXISTS zbr_usercols (\
    name TEXT NOT NULL,\
    user_name TEXT,\
    collation INTEGER NOT NULL,\
    PRIMARY KEY (name)\
)\
"

// Weighting definitions
#define SQLITE_TABLEDEF_WEIGHTING   "\
CREATE TABLE IF NOT EXISTS zbr_weighting (\
    colname TEXT NOT NULL,\
    transform INTEGER NOT NULL,\
    weight REAL NOT NULL\
)\
"
// All table defs to assert
#define SQLITE_TABLEDEFS_ALL    SQLITE_TABLEDEF_DBVER, SQLITE_TABLEDEF_PROPERTY, SQLITE_TABLEDEF_USERCOLS SQLITE_TABLEDEF_WEIGHTING

#endif // SQLDEFS_H
