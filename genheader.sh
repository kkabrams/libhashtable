#!/bin/sh
cat libhashtable.c | grep -A 100 _A_ | grep -B 100 _B_ | grep -v "_[AB]_" > hashtable.h
cat libhashtable.c | grep '(.*) *{' | egrep -v 'if|for|while' | sed 's/ {/;/' >> hashtable.h
