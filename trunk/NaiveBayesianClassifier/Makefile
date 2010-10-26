classifier: main.o BayesianClassifier.o PrintTemplates.o
	g++ BayesianClassifier.o main.o -o classifier
BayesianClassifier.o: BayesianClassifier.cpp BayesianClassifier.h
	g++ -c BayesianClassifier.cpp
PrintTemplates.o:
	g++ -c PrintTemplates.cpp
main.o: main.cpp BayesianClassifier.h PrintTemplates.cpp
	g++ -c main.cpp
clean:
	rm -f classifier main.o BayesianClassifier.o PrintTemplates.o
