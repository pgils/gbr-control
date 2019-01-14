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
    eui64       INTEGER PRIMARY KEY,
    ipaddress   TEXT NOT NULL,
    active      INTEGER NOT NULL,
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
    group_id    INTEGER PRIMARY KEY,
    groupname   TEXT NOT NULL UNIQUE
);

CREATE TABLE tblGroup_node (
    group_id    INTEGER,
    node_eui64  INTEGER,
    FOREIGN KEY (group_id) REFERENCES tblGroup (group_id)
    ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (node_eui64) REFERENCES tblNode (eui64)
    ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY (group_id, node_eui64)
);

INSERT INTO tblStatus (statusdesc)
VALUES
('uninitialized'), ('initialized');

INSERT INTO tblRole (rolename)
VALUES
('actuator'), ('sensor');

INSERT INTO tblGroup (groupname)
VALUES
('A'), ('B'), ('C'), ('D'), ('E'), ('F');

INSERT INTO tblSignal (signalname)
VALUES
('on'), ('off'), ('blink');
