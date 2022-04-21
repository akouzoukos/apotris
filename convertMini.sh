
BASE_PATH="./sprites/small/"
OUTPUT="./data/"

counter=0
for f in $BASE_PATH
do
    for sprite in ${f}*.png
    do
        ((counter++))
        ./superfamiconv -v -i "$sprite" -p "$OUTPUT"small.bin -t "$OUTPUT"small"$counter"tiles.bin -m "$OUTPUT"small"$counter"map.bin -M gba -R -D
    done
done