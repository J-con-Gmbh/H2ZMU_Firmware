#!/bin/bash

while true; do
    read -p "Reset database migrations.\nAfter restart of firmware all data will be lost! " yn
    case $yn in
        [Yy]* ) sqlite3 ../db/h2zmu.db "DELETE FROM 'migrations';"; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done


