make clean
make


SCHEME="gshare"
PARMS=$1

# Testing all traces
echo "\n\nBranch prediction scheme: $SCHEME with parms: $PARMS"
for i in "fp_1.bz2" "fp_2.bz2" "int_1.bz2" "int_2.bz2" "mm_1.bz2" "mm_2.bz2"; 
    do echo "\n$i"; 
    eval "bunzip2 -kc ../traces/$i | ./predictor --$SCHEME:$PARMS"
done

