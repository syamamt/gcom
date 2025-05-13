#!/bin/bash

tc qdisc add dev eth0 root netem loss ${1}% delay ${2}ms