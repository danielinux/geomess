for i in `seq 1 20` ; do
    ./picomesh $i `expr $RANDOM % 80` `expr $RANDOM % 80` 16 10 &>/dev/null &
done
