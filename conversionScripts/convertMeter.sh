
BASE_PATH="./sprites/meter/"
OUTPUT="./data/"

for f in $BASE_PATH
do
    for sprite in ${f}*.png
    do
        name=${sprite##*/}
        ./superfamiconv -v -i "$sprite" -t "$OUTPUT${name%.*}"_tiles.bin -M gba -R -D
    done
done
