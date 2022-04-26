
BASE_PATH="./sprites/title/"
OUTPUT="./data/"

counter=0
for f in $BASE_PATH
do
    for sprite in ${f}*.png
    do
        ((counter++))
        ./superfamiconv -v -i "$sprite" -p "$OUTPUT"title_pal.bin -t "$OUTPUT"title"$counter"tiles.bin -M gba -D -S
    done
done