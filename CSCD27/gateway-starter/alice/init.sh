#!/usr/bin/env bash

ip route del default 
ip route add default via 10.0.0.3
node index.js