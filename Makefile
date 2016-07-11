app:
	gcc main.cpp Arff.cpp DecisionTree.cpp Hereboy.cpp inih/ini.c inih/cpp/INIReader.cpp -I "./tclap-1.2.1/include" -I "./inih/cpp" -Ofast -o "../../tools/hereboy/hereboy"
