# LZW_decompressor

[LZW from Wikipedia](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Welch)

Decompressor for LZW
 * 12 bits symboles
 * The dictonary is 4096 deep
 * Dictionary initialised from 0 to 255 by its index

Reading from stdin, writing to stdout.
``` bash
./un_lzw < compressed_file > uncompressed_file
```
