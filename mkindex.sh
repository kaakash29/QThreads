#!/bin/bash

ROOT=$PWD/
HTTP="/"
OUTPUT="index.html" 

i=0
echo "<UL>" > $OUTPUT
for filepath in `find "$ROOT" -maxdepth 1 -mindepth 1 | sort`; do
  path=`basename "$filepath"`
  if [ -d $filepath ] ; then
    echo "  <LI>$path</LI>" >> $OUTPUT
  else
    echo "  <LI><a href=\"/$path\">$path</a></LI>" >> $OUTPUT
  fi
  echo "  <UL>" >> $OUTPUT
  for i in `find "$filepath" -maxdepth 1 -mindepth 1 -type f| sort`; do
    file=`basename "$i"`
    echo "    <LI><a href=\"/$path/$file\">$file</a></LI>" >> $OUTPUT
  done
  echo "  </UL>" >> $OUTPUT
done
echo "</UL>" >> $OUTPUT
