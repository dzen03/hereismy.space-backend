path=db/photos

./clean_db.sh

for X in $path/*.jpg;
do
    # do convert "$X" -resize 602x400 -strip -quality 86 "$X";
    photo="${X%*.*}"
    photo_ext="${X##*.}"
    echo "$X" "->" "$photo.XX.webp"
    magick "$X" -auto-orient -resize 500x500 -strip -quality 50 "$photo.sm.webp"
    magick "$X" -auto-orient -resize 1000x1000 -strip -quality 50 "$photo.md.webp"
    magick "$X" -auto-orient -quality 50 "$photo.lg.webp"
done
