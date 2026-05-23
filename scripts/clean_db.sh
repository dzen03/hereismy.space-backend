paths=(
    "./db/365/photos"
    "./db/portfolio/photos"
)

for path in "${paths[@]}"; do
  rm $path/*.webp
done
