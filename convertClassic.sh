
BASE_PATH="./sprites/classic/"
OUTPUT="./data/"

counter=0
for f in $BASE_PATH
do
    for sprite in ${f}*.png
    do
        ((counter++))
        ./superfamiconv -v -i "$sprite" -p "$OUTPUT"classic_pal.bin -t "$OUTPUT"classic"$counter"tiles.bin -M gba -R -D
    done
done
