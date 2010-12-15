include mkinc.mak
CFLAGS=-Iinclude
.PHONY: source-compile clean
classifier: source-compile
	g++ -I. $(nbc_sources:.c=.o) -L. $(nbc_libs) -o classifier
source-compile: $(nbc_sources) $(nbc_headers)
	g++ -c -I. $(nbc_sources)
clean:
	rm -f classifier *.o
