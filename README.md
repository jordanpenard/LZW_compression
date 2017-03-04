# LZW compression

[LZW from Wikipedia](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Welch)

Implementation:
 * 12 bits symboles
 * The dictonary is 4096 deep
 * Dictionary initialised from 0 to 255 by its index

## Decompressor
Reading from stdin, writing to stdout.
``` bash
make decompressor
./lzw_decompressor < compressed_file > uncompressed_file
```

## Compressor
Reading from stdin, writing to stdout.
``` bash
make compressor
./lzw_compressor < uncompressed_file > compressed_file
```
