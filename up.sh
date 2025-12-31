#!/usr/bin/env bash

a1="$1"
host="${a1:=h183}"
./build.sh arm && tar -C bin -cvzf - cuckoo | ssh root@${host} "tar -xvzf -"
