for i in `seq 1 20` ; do
    ./picomesh $i `expr $RANDOM % 40` `expr $RANDOM % 40` 16 10 &>/dev/null &
done
