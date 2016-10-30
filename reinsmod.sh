#!/bin/sh

mod="$1"

rmmod $1
insmod $1
