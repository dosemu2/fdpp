#!/bin/sh

parsers/thunk_gen 1 <$1 | sort | uniq | autom4te -l m4sugar $2/thunks.m4 -
