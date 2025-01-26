# Backend for my site

## Links

* Hosted version: [hereismy.space](https://hereismy.space).
* Using self-written server: [Simple HTTP server](https://github.com/dzen03/simple-http-server.git)
* [Frontend](https://github.com/dzen03/hereismy.space-frontend.git)
* [Docker build](https://github.com/dzen03/hereismy.space.git)

## Cloning

```bash
git clone --recurse-submodules https://github.com/dzen03/hereismy.space-backend.git
```

## Building

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Using (docker version)
See [this repo](https://github.com/dzen03/hereismy.space.git)

## Using (standalone version)
0. Install [ImageMagick](https://imagemagick.org/script/index.php).
1. Place your photos into `db/photos`,
metadata into `db/metadata`
and built site into `staticfiles`.
Metadata format:
```
{tag1}#{tag2}#...#{tagN}
{description}
```
2. Run `make_db.sh` to create database.
3. Run `./build/here_is_my_space`.
