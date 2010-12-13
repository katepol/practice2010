include mkinc.mak
CFLAGS=-Iinclude
classifier: source-compile
	g++ $(nbc_sources:.c=.o) -L. -lcurl -L. -lstemmer -o classifier
source-compile: $(nbc_sources) $(nbc_headers)
	g++ -c -I. $(nbc_sources)
clean:
	rm -f classifier *.o
