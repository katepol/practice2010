dloader: NotEpubException.o CurlDownload.o
	g++ NotEpubException.o CurlDownload.o -L. -lcurl -o dloader
NotEpubException.o: NotEpubException.cpp NotEpubException.h
	g++ -c NotEpubException.cpp
CurlDownload.o: CurlDownload.cpp NotEpubException.h
	g++ -c CurlDownload.cpp
clean:
	rm -f
