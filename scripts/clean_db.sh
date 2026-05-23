paths=(
    "./db/challenge/photos"
    "./db/portfolio/photos"
)

for path in "${paths[@]}"; do
  rm $path/*.webp
done
