#!/bin/sh

TSTAMP=.tstamp
DATE=`git log -1 --format=%cd --date=rfc`
if [ $? != 0 ]; then
    echo "Non-git builds deprecated" >&2
    exit 1
fi
if ! touch --date="$DATE" $TSTAMP 2>/dev/null; then
    echo "touch doesnt support --date, build may be incomplete" >&2
fi
echo $TSTAMP
