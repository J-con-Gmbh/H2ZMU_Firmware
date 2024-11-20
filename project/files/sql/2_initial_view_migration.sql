/* VIEW MIGRATION INITIAL */

DROP VIEW IF EXISTS v_errors;
CREATE VIEW v_errors AS
SELECT 'e'.'id', 'e'.'errorcode', t1.short AS shortdescr, t2.short AS possiblecause, t3.short AS fixby
FROM 'errors' e
         INNER JOIN 'textblocks' t1 ON e.fk_shortdescr = t1.'short'
         INNER JOIN 'textblocks' t2 ON e.fk_possiblecause = t2.'short'
         INNER JOIN 'textblocks' t3 ON e.fk_fixby = t3.'short';

DROP VIEW IF EXISTS v_warnings;
CREATE VIEW v_warnings AS
SELECT 'w'.'id', 'w'.'warningcode', t1.short AS shortdescr, t2.short AS possiblecause_de
FROM 'warnings' w
         INNER JOIN 'textblocks' t1 ON w.fk_shortdescr = t1.'short'
         INNER JOIN 'textblocks' t2 ON w.fk_possiblecause = t2.'short';

DROP VIEW IF EXISTS v_params;
CREATE VIEW v_params AS
SELECT 'p'.'id', 'p'.'nr', 'p'.'shortdescr', ps.value, dt.name as datatype
    FROM (SELECT fk_param, fk_user, max(timeset), value from paramstate group by fk_param) ps
             JOIN params p on ps.fk_param = p.nr
             JOIN datatypes dt ON p.datatype = dt.id;

DROP VIEW IF EXISTS v_errors_translations_mapped;
CREATE VIEW v_errors_translations_mapped AS
SELECT e.id AS 'id',
       e.errorcode AS 'errorcode',
       t1.id AS 'fk_shortdescr',
       t2.id AS 'fk_fixby',
       t3.id AS 'fk_possiblecause',
       e.globalrelevance AS 'globalrelevance'
FROM errors e
         INNER JOIN textblocks t1 ON e.fk_shortdescr = t1.short
         INNER JOIN textblocks t2 ON e.fk_fixby = t2.short
         INNER JOIN textblocks t3 ON e.fk_possiblecause = t3.short;

DROP VIEW IF EXISTS v_warnings_translations_mapped;
CREATE VIEW v_warnings_translations_mapped AS
SELECT w.id AS 'id',
       w.warningcode AS 'warningcode',
       t1.id AS 'fk_shortdescr',
       t3.id AS 'fk_possiblecause',
       w.globalrelevance AS 'globalrelevance'
FROM warnings w
         INNER JOIN textblocks t1 ON w.fk_shortdescr = t1.short
         INNER JOIN textblocks t3 ON w.fk_possiblecause = t3.short;

DROP VIEW IF EXISTS v_params_translations_mapped;
CREATE VIEW v_params_translations_mapped AS
SELECT p.id AS 'id',
       p.nr AS 'nr',
       p.shortdescr AS 'shortdescr',
       t1.id AS 'fk_description',
       p.datatype AS 'datatype',
       p.unit AS unit,
       p.rolerestriction AS 'rolerestriction',
       p.switchrestriction AS 'switchrestriction',
       t2.id AS 'fk_errormsg',
       p.fk_hardwareinterface as 'fk_hardwareinterface'
FROM params p
         INNER JOIN textblocks t1 on p.fk_description = t1.short
         INNER JOIN textblocks t2 on p.fk_errormsg = t2.short;

DROP VIEW IF EXISTS v_cascadestate;
CREATE VIEW v_cascadestate AS
SELECT p.id AS 'id',
        p.nr AS 'nr',
        p.shortdescr AS 'shortdescr',
        t1.id AS 'fk_description',
        p.datatype AS 'datatype',
        p.unit AS unit,
       p.rolerestriction AS 'rolerestriction',
        p.switchrestriction AS 'switchrestriction',
        t2.id AS 'fk_errormsg',
        p.fk_hardwareinterface as 'fk_hardwareinterface'
FROM params p
         INNER JOIN textblocks t1 on p.fk_description = t1.short
         INNER JOIN textblocks t2 on p.fk_errormsg = t2.short;