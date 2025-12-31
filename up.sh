#!/usr/bin/env bash

host="${1:h183}"
./build.sh arm && tar -C bin -cvzf - cuckoo | ssh root@${host} "tar -xvzf -"
