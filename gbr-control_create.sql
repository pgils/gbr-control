CREATE TABLE tblStatus (
    status_id   INTEGER PRIMARY KEY,
    statusdesc  TEXT NOT NULL UNIQUE
);

CREATE TABLE tblRole (
    role_id     INTEGER PRIMARY KEY,
    rolename    TEXT NOT NULL UNIQUE
);

CREATE TABLE tblSignal (
    signal_id   INTEGER PRIMARY KEY,
    signalname  TEXT NOT NULL UNIQUE
);

CREATE TABLE tblNode (
    eui64_id    INTEGER PRIMARY KEY,
    eui64       TEXT NOT NULL,
    active      INTEGER DEFAULT 0,
    status_id   INTEGER NOT NULL,
    role_id     INTEGER NOT NULL,
    signal_id   INTEGER DEFAULT 0,
    FOREIGN KEY (status_id) REFERENCES tblStatus (status_id)
    ON DELETE NO ACTION ON UPDATE CASCADE,
    FOREIGN KEY (role_id) REFERENCES tblRole (role_id)
    ON DELETE NO ACTION ON UPDATE CASCADE,
    FOREIGN KEY (signal_id) REFERENCES tblSignal (signal_id)
    ON DELETE SET DEFAULT ON UPDATE CASCADE
);

CREATE TABLE tblGroup (
    group_id    INTEGER PRIMARY KEY
);

CREATE TABLE tblGroup_node (
    group_id    INTEGER,
    eui64_id    INTEGER,
    FOREIGN KEY (group_id) REFERENCES tblGroup (group_id)
    ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (eui64_id) REFERENCES tblNode (eui64_id)
    ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY (group_id, eui64_id)
);

INSERT INTO tblStatus (statusdesc)
VALUES
('uninitialized'), ('initialized');

INSERT INTO tblRole (rolename)
VALUES
('actuator'), ('sensor');

INSERT INTO tblSignal (signalname)
VALUES
('none'), ('off'), ('red'), ('green'), ('blue');
