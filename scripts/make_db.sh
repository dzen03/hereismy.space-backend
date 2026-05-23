paths=(
    "./db/challenge/photos"
    "./db/portfolio/photos"
)

process_photo() {
    local X="$1"
    local photo="${X%.*}"

    echo "Converting: $X -> $photo.{sm,md,lg}.webp"
    magick "$X" -auto-orient -resize 500x500 -strip -quality 50 "$photo.sm.webp"
    magick "$X" -auto-orient -resize 1000x1000 -strip -quality 50 "$photo.md.webp"
    magick "$X" -auto-orient -quality 50 "$photo.lg.webp"
}

if [ -n "$1" ]; then
    if [ -f "$1" ] && [[ "$1" == *.jpg ]]; then
        echo "Updating single photo: $1"
        process_photo "$1"
        echo "Done!"
        exit 0
    else
        echo "Error: File '$1' does not exist or is not a .jpg file."
        exit 1
    fi
fi

echo "Cleaning up existing .webp files..."
for path in "${paths[@]}"; do
    if [ -d "$path" ]; then
        rm -f "$path"/*.webp 2>/dev/null
    fi
done

if [ -f "./scripts/clean_db.sh" ]; then
    ./scripts/clean_db.sh
fi

echo "Starting full conversion..."
for path in "${paths[@]}"; do
    echo "Processing directory: $path"

    for X in "$path"/*.jpg; do
        [ -f "$X" ] || continue
        process_photo "$X"
    done
done

echo "Full update complete!"
