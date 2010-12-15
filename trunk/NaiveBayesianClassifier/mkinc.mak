nbc_sources= \
	BayesianClassifier.cpp \
	Downloader.cpp \
	Parser.cpp \
	main.cpp

nbc_headers= \
	BayesianClassifier.h \
	Downloader.h \
	HtmlToXml.h \
	Parser.h \
	PrintTemplates.h \
	StringConvert.h \
	externallibs/include/curl/curl.h \
	externallibs/include/curl/types.h \
	externallibs/include/libstemmer.h \
	externallibs/include/expat/expat.h \
	externallibs/include/tidy/tidy.h \
	externallibs/include/tidy/buffio.h

nbc_libs = \
	externallibs/lib/libtidy.a \
	externallibs/lib/libcurl.so \
	externallibs/lib/libexpat.so \
	externallibs/lib/libstemmer.so
