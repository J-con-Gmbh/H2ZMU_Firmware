INSERT INTO sensors ( id,
                      type,
                      type_order,
                      serialnumber,
                      name,
                      manufacturer,
                      uppermeasurelimit_manufacturer,
                      lowermeasurelimit_manufacturer,
                      fk_hardwareprotocol,
                      hardwareprotocol_address,
                      offset)
VALUES  ('0', '2', '1', 'pcascade1_11', 'p2', 'riga', '300', '0', '4', '0', '0'),
        ('1', '1', '1', 'tcascade1_11', 't1', 'riga', '300', '0', '4', '0', '0');

INSERT INTO cascades ( id,
                       fk_sensor_lower,
                       fk_sensor_upper)
VALUES ('1', '0', '0');

INSERT INTO bottles ( serialnumber,
                      fk_cascade,
                      cascade_order,
                      vol_0,
                      vol_ref,
                      pressure_0,
                      pressure_ref,
                      fk_sensor,
                      tara,
                      manufacturer,
                      builtyear,
                      nextcheck
)
VALUES ('btl1_1', '1', '1', '341.9', '351.76', '1', '477', '0', '0', NULL, '2000', '2035');
