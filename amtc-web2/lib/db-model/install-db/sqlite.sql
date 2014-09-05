
-- amtc-web SQLite schema dump

-- notifications: Short messages for dashboard
CREATE TABLE notifications (
  id                INTEGER PRIMARY KEY, 
  tstamp            DATETIME DEFAULT CURRENT_TIMESTAMP, 
  ntype             VARCHAR(12), 
  message           VARCHAR(64)
);

-- rooms as containers for groups of clients
CREATE TABLE rooms (
  id                INTEGER PRIMARY KEY, 
  roomname          VARCHAR(32), 
  seats             INTEGER
);

-- organizational units
CREATE TABLE ous (
  id                INTEGER      NOT NULL PRIMARY KEY AUTOINCREMENT,
  parent            INTEGER      NULL,
  name              VARCHAR(128) NOT NULL,
  description       VARCHAR(255)
);

-- amt(c) option sets
CREATE TABLE optionsets (
  id                INTEGER      NOT NULL PRIMARY KEY AUTOINCREMENT,
  name              VARCHAR(128) NOT NULL,
  description       VARCHAR(128),
  sw_v5             INTEGER,
  sw_dash           INTEGER,
  sw_scan22         INTEGER,
  sw_scan3389       INTEGER,
  sw_usetls         INTEGER,
  sw_skipcertchk    INTEGER,
  opt_maxthreads    INTEGER,
  opt_timeout       INTEGER,
  opt_passfile      VARCHAR(128),
  opt_cacertfile    VARCHAR(128)
);

