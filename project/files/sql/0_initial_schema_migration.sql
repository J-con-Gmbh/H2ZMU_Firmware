DROP TABLE IF EXISTS 'devicestatus';
CREATE TABLE IF NOT EXISTS 'devicestatus' ('id' INT,
                                           'name' VARCHAR[50],
                                           'value' TEXT);

DROP TABLE IF EXISTS 'migrations';
CREATE TABLE IF NOT EXISTS 'migrations' ('version' INTEGER NOT NULL,
                                         'fname' TEXT NOT NULL,
                                         PRIMARY KEY('version'));

DROP TABLE IF EXISTS 'usergroups';
CREATE TABLE IF NOT EXISTS 'usergroups' ('id' INTEGER NOT NULL,
                                         'name' INTEGER NOT NULL,
                                         PRIMARY KEY('id'));

DROP TABLE IF EXISTS 'users';
CREATE TABLE IF NOT EXISTS 'users' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                    'username' TEXT NOT NULL UNIQUE,
                                    'passwordhash' TEXT NOT NULL,
                                    'fk_usergroup' INTEGER NOT NULL,
                                    'lastlogin' INTEGER, 'deleted' BOOL NOT NULL DEFAULT FALSE,
                                    FOREIGN KEY('fk_usergroup') REFERENCES 'usergroups'('id'));



DROP TABLE IF EXISTS 'textblocks';
CREATE TABLE IF NOT EXISTS 'textblocks' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                           'short' VARCHAR(10) NOT NULL UNIQUE
    );

DROP TABLE IF EXISTS 'translations';
CREATE TABLE IF NOT EXISTS 'translations' (
    'id' INTEGER NOT NULL,
    'loc' VARCHAR(2),
    'trans' TEXT,

    UNIQUE(id, loc)
    );



DROP TABLE IF EXISTS 'hardwareinterfaces';
CREATE TABLE IF NOT EXISTS 'hardwareinterfaces' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

DROP TABLE IF EXISTS 'hardwareprotocols';
CREATE TABLE IF NOT EXISTS 'hardwareprotocols' ('id' INTEGER NOT NULL,
                                                'name' VARCHAR(20) NOT NULL,
    'rawvaluemax' FLOAT NOT NULL,
    'rawvaluemin' FLOAT NOT NULL,
    'fk_hardwareinterfaces' INT,
    FOREIGN KEY('fk_hardwareinterfaces') REFERENCES 'hardwareinterfaces'('id')
    );

DROP TABLE IF EXISTS 'datatypes';
CREATE TABLE IF NOT EXISTS 'datatypes' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                        'name' TEXT NOT NULL);

DROP TABLE IF EXISTS 'params';
CREATE TABLE IF NOT EXISTS 'params' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                     'nr' INT NOT NULL UNIQUE,
                                     'shortdescr' TEXT NOT NULL,
                                     'fk_description' INT NOT NULL,
                                     'datatype' INT NOT NULL,
                                     'unit' TEXT,
                                     'rolerestriction' INT,
                                     'switchrestriction' INT DEFAULT 0,
                                     'fk_errormsg' VARCHAR(10) DEFAULT 'blnk',
    'fk_hardwareinterface' INT,
    FOREIGN KEY('fk_description') REFERENCES 'translations'('id'),
    FOREIGN KEY('fk_errormsg') REFERENCES 'translations'('id'),
    FOREIGN KEY('fk_hardwareinterface') REFERENCES 'hardwareinterfaces'('id'));

DROP TABLE IF EXISTS 'paramstate';
CREATE TABLE IF NOT EXISTS 'paramstate' ('fk_param' INT,
                                         'fk_user' INTEGER NOT NULL,
                                         'timeset' TEXT NOT NULL,
                                         'value' TEXT NOT NULL,
                                         FOREIGN KEY('fk_param') REFERENCES 'params'('nr'),
    FOREIGN KEY('fk_user') REFERENCES 'users'('id'));


DROP TABLE IF EXISTS 'errors';
CREATE TABLE IF NOT EXISTS 'errors' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                     'errorcode' VARCHAR(10),
    'fk_shortdescr' VARCHAR(10) NOT NULL,
    'fk_fixby' INT NOT NULL,
    'fk_possiblecause' INT NOT NULL,
    'globalrelevance' BOOL DEFAULT TRUE,
    FOREIGN KEY('fk_shortdescr') REFERENCES 'translations'(short),
    FOREIGN KEY('fk_fixby') REFERENCES 'translations'(id),
    FOREIGN KEY('fk_possiblecause') REFERENCES 'translations'(id));


DROP TABLE IF EXISTS 'occurrederrors';
CREATE TABLE IF NOT EXISTS 'occurrederrors' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                             'fk_error' INT NOT NULL,
                                             'occurred_timestamp' TEXT NOT NULL,
                                             'fk_hardwareinterface' INT,
                                             'fk_user' INT,
                                             'resolved_timestamp' TEXT,
    FOREIGN KEY('fk_error') REFERENCES 'errors'(id),
    FOREIGN KEY('fk_hardwareinterface') REFERENCES 'hardwareinterfaces'(id),
    FOREIGN KEY('fk_user') REFERENCES 'users'(id));


DROP TABLE IF EXISTS 'warnings';
CREATE TABLE IF NOT EXISTS 'warnings' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                        'warningcode' VARCHAR(10),
                                        'fk_shortdescr' VARCHAR(10) NOT NULL,
                                        'fk_possiblecause' INT NOT NULL,
                                        'globalrelevance' BOOL DEFAULT TRUE,
    FOREIGN KEY('fk_shortdescr') REFERENCES 'translations'(short),
    FOREIGN KEY('fk_possiblecause') REFERENCES 'translations'(id));


DROP TABLE IF EXISTS 'occurredwarnings';
CREATE TABLE IF NOT EXISTS 'occurredwarnings' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                             'fk_warning' INT NOT NULL,
                                             'occurred_timestamp' TEXT NOT NULL,
                                             'fk_hardwareinterface' INT,
                                             'fk_user' INT,
                                             'resolved_timestamp' TEXT,
    FOREIGN KEY('fk_warning') REFERENCES 'warnings'(id),
    FOREIGN KEY('fk_hardwareinterface') REFERENCES 'hardwareinterfaces'(id),
    FOREIGN KEY('fk_user') REFERENCES 'users'(id));


DROP TABLE IF EXISTS 'sensors';
CREATE TABLE IF NOT EXISTS 'sensors' ('id' INTEGER PRIMARY KEY NOT NULL,
                                      'type' INT NOT NULL,
                                      'type_order' INT NOT NULL,
                                      'serialnumber' VARCHAR(50) NOT NULL,
    'name' VARCHAR(50) NOT NULL,
    'manufacturer' VARCHAR(50) NOT NULL,
    'uppermeasurelimit_manufacturer' INT NOT NULL,
    'lowermeasurelimit_manufacturer' INT NOT NULL,
    'fk_hardwareprotocol' INT NOT NULL,
    'hardwareprotocol_address' INT NOT NULL,
    'offset' FLOAT DEFAULT '0',
    FOREIGN KEY('fk_hardwareprotocol') REFERENCES 'hardwareprotocols'(id),
    UNIQUE('type','type_order')
    );


DROP TABLE IF EXISTS 'cascades';
CREATE TABLE IF NOT EXISTS 'cascades' ('id' INTEGER PRIMARY KEY,
                                       'fk_sensor_lower' INT,
                                       'fk_sensor_upper' INT NOT NULL,
                                       FOREIGN KEY('fk_sensor_lower') REFERENCES 'sensors'(id),
    FOREIGN KEY('fk_sensor_upper') REFERENCES 'sensors'(id)
    );


DROP TABLE IF EXISTS 'bottles';
CREATE TABLE IF NOT EXISTS 'bottles' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                      'serialnumber' VARCHAR(50) NOT NULL,
    'fk_cascade' INT NOT NULL,
    'cascade_order' INT NOT NULL,
    'fk_sensor' INT,
    'tara' FLOAT,
    'manufacturer' VARCHAR(50),
    'builtyear' VARCHAR(4),
    'nextcheck' VARCHAR(6),
    'vol_0' FLOAT,
    'vol_ref' FLOAT,
    'pressure_0' FLOAT,
    'pressure_ref' FLOAT,
    FOREIGN KEY('fk_sensor') REFERENCES 'sensors'(id),
    FOREIGN KEY('fk_cascade') REFERENCES 'cascades'(id),
    UNIQUE('fk_cascade', 'cascade_order')
    );

DROP TABLE IF EXISTS 'vesselstate';
CREATE TABLE IF NOT EXISTS 'vesselstate' ('id' INTEGER PRIMARY KEY);

DROP TABLE IF EXISTS 'cascadestate';
CREATE TABLE IF NOT EXISTS 'cascadestate' ('id' INTEGER PRIMARY KEY NOT NULL,
                                           'fk_vesselstate' INT NOT NULL,
                                           'fk_cascade' INT NOT NULL,
                                           'nm3' FLOAT NOT NULL,
                                           FOREIGN KEY('fk_vesselstate') REFERENCES 'vesselstate'(id),
    FOREIGN KEY('fk_cascade') REFERENCES 'cascades'(id)
    );

DROP TABLE IF EXISTS 'sensorstate';
CREATE TABLE IF NOT EXISTS 'sensorstate' ('id' INTEGER PRIMARY KEY NOT NULL,
                                          'fk_sensor' INT NOT NULL,
                                          'fk_cstate' INT NOT NULL,
                                          'value' FLOAT NOT NULL,
                                          'value_raw' FLOAT NOT NULL,
                                          'timestamp' TEXT NOT NULL,
                                          FOREIGN KEY('fk_sensor') REFERENCES 'sensors'(id),
    FOREIGN KEY('fk_cstate') REFERENCES 'cascadestate'(id)
    );

DROP TABLE IF EXISTS 'measurement';
CREATE TABLE IF NOT EXISTS 'measurement' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                          'fk_user' INT,
                                          'ts_start' TEXT NOT NULL,
                                          'ts_end' TEXT,
                                          'ext_measure_id' TEXT,
                                          'fk_vesselstate_start' INT,
                                          'fk_vesselstate_end' INT,
                                          'valid' BOOL NOT NULL,
                                          FOREIGN KEY('fk_user') REFERENCES 'users'(id),
    FOREIGN KEY('fk_vesselstate_start') REFERENCES 'vesselstate'(id),
    FOREIGN KEY('fk_vesselstate_end') REFERENCES 'vesselstate'(id)
    );

DROP TABLE IF EXISTS 'archive';
CREATE TABLE IF NOT EXISTS 'archive' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                                     'nr' INT NOT NULL UNIQUE,
                                     'shortdescr' TEXT NOT NULL,
                                     'fk_description' INT NOT NULL,
                                     'datatype' INT NOT NULL,
                                     'unit' TEXT,
                                     'rolerestriction' INT,
                                     'switchrestriction' INT DEFAULT 0,
                                     'fk_errormsg' VARCHAR(10) DEFAULT 'blnk',
    'fk_hardwareinterface' INT,
    FOREIGN KEY('fk_description') REFERENCES 'translations'('id'),
    FOREIGN KEY('fk_errormsg') REFERENCES 'translations'('id'),
    FOREIGN KEY('fk_hardwareinterface') REFERENCES 'hardwareinterfaces'('id'));
