
BASE_PATH="./sprites/moves/"
OUTPUT="./data/"

counter=0
for f in $BASE_PATH
do
    for sprite in ${f}*.png
    do
        ((counter++))
        ./superfamiconv -v -i "$sprite" -t "$OUTPUT"move"$counter"tiles.bin -m "$OUTPUT"move"$counter"map.bin -M gba -R -D
    done
done
