for i in `seq 1 100` ; do
    ./picomesh $i `expr $RANDOM % 100` `expr $RANDOM % 100` 10 10 &>/dev/null &
done
