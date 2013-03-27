all:
	g++ -shared -std=c++0x src/config.h
	g++ -shared -std=c++0x src/common.h	
	g++ -shared -std=c++0x src/delay.h
	g++ -shared -std=c++0x src/envelope.h
	g++ -shared -std=c++0x src/filter.h
	g++ -shared -std=c++0x src/oscillator.h