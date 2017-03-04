compressor:
	gcc -O2 -g lzw_compressor.c -o lzw_compressor

decompressor:
	gcc -O2 -g lzw_decompressor.c -o lzw_decompressor

all: compressor decompressor
