#!/usr/bin/env bash

SCRIPT_PATH="./conversionScripts/"

for f in $SCRIPT_PATH
do
    for script in ${f}*.sh
    do
        bash "$script"
    done
done
