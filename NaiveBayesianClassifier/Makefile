classifier: main.o BayesianClassifier.o Downloader.o Checkers.o WrongFileException.o PrintTemplates.o
	g++ Checkers.o WrongFileException.o PrintTemplates.o Downloader.o BayesianClassifier.o main.o -L. -lcurl -o classifier
main.o: main.cpp BayesianClassifier.h Downloader.h
	g++ -c main.cpp
BayesianClassifier.o: BayesianClassifier.cpp BayesianClassifier.h PrintTemplates.cpp
	g++ -c PrintTemplates.cpp BayesianClassifier.cpp
PrintTemplates.o:
	g++ -c PrintTemplates.cpp
Downloader.o: StringConvert.h WrongFileException.h Downloader.h Checkers.h Downloader.cpp
	g++ -c Downloader.cpp
Checkers.o: WrongFileException.h Checkers.h Checkers.cpp
	g++ -c Checkers.cpp
WrongFileException.o: WrongFileException.cpp WrongFileException.h
	g++ -c WrongFileException.cpp
clean:
	rm -f classifier main.o BayesianClassifier.o PrintTemplates.o Downloader.o Checkers.o WrongFileException.o
