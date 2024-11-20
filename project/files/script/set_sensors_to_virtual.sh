#!/bin/bash

sqlite3 ../db/h2zmu.db "UPDATE sensors SET fk_hardwareprotocol = '2';"
sqlite3 ../db/h2zmu.db "select * from sensors;"
