#!/usr/bin/env bash

set -e
set -o pipefail

parsers/thunk_gen 1 <$1 | sort | uniq | autom4te -l m4sugar $2/thunks.m4 -
parsers/thunk_gen 2 <$1
