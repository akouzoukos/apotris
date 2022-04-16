
BASE_PATH="./sprites/"
OUTPUT="./data/"

counter = 0
for f in $BASE_PATH
do
    for sprite in ${f}*.png
    do
        ((counter++))
        ./superfamiconv -v -i "$sprite" -p "$OUTPUT"sprite_pal.bin -t "$OUTPUT"sprite"$counter"tiles.bin -m "$OUTPUT"sprite"$counter"map.bin -M gba -R
    done
done