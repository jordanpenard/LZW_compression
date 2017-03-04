all: compressor decompressor

compressor:
	gcc -O2 -g lzw_compressor.c -o lzw_compressor

decompressor:
	gcc -O2 -g lzw_decompressor.c -o lzw_decompressor

clean:
	rm -rf lzw_compressor.dSYM/ lzw_decompressor.dSYM/ lzw_compressor lzw_decompressor
