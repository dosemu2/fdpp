while read -a line_a; do
    line=${line_a[@]}
    [ -z "$line" ] && continue
    SEG=`echo $line | cut -d " " -f 3`
    [ -z "$SEG" ] && continue
    nb=${#line_a[@]}
    SYM=${line_a[$((nb - 1))]}
    GRP=`grep " $SEG" $1 | grep "^group " | expand | tr -s "[:space:]" | cut -d " " -f 2`
    [ -z "$GRP" ] && continue
    echo %define _SEG_$SYM $GRP
done
