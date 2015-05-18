#!/bin/sh
L=$(grep -c '^.*$' libhashtable.c)
cat libhashtable.c | grep -A "$L" _A_ | grep -B "$L" _B_ | grep -v "_[AB]_" > hashtable.h
cat libhashtable.c | grep '(.*) *{' | egrep -v 'switch|if|for|while' | sed 's/ {/;/' >> hashtable.h
