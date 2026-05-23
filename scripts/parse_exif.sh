if [ -n "$1" ]; then
    if [ -f "$1" ] && [[ "$1" == *.jpg ]]; then
        exiftool -json -DateTimeOriginal -Make -Model -LensID -FocalLength -Aperture -ShutterSpeed -ISO -ImageSize $1
        exit 0
    else
        >&2 echo "Error: File '$1' does not exist or is not a .jpg file."
        exit 1
    fi
fi

>&2 echo "Error: Should pass file"
exit 1
