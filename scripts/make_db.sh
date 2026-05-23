paths=(
    "./db/365/photos"
    "./db/portfolio/photos"
)

./clean_db.sh

for path in "${paths[@]}"; do
    echo "Processing directory: $path"

    for X in "$path"/*.jpg; do
        [ -f "$X" ] || continue

        photo="${X%.*}"

        echo "$X" "->" "$photo.XX.webp"
        magick "$X" -auto-orient -resize 500x500 -strip -quality 50 "$photo.sm.webp"
        magick "$X" -auto-orient -resize 1000x1000 -strip -quality 50 "$photo.md.webp"
        magick "$X" -auto-orient -quality 50 "$photo.lg.webp"
    done
done
