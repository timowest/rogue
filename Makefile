BUNDLE = lv2-rogue.lv2
INSTALL_DIR = /usr/local/lib/lv2

SOURCES = dsp/*.cpp src/*.cpp
SOURCES_UI = dsp/*.cpp src/gui/rogue-gui.cpp
FLAGS = -fPIC -DPIC -std=c++11 
FAST = -Ofast -ffast-math
LIBSRC = `pkg-config --cflags --libs samplerate`
LVTK = `pkg-config --cflags --libs lvtk-plugin-1`
LVTK_UI = `pkg-config --cflags --libs lvtk-ui-1`
QT = `pkg-config --cflags --libs QtGui` 
FFTW = -lfftw3f
SNDFILE = -lsndfile

$(BUNDLE): manifest.ttl rogue.ttl presets.ttl rogue.so rogue-gui.so presets styles
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp -r $^ $(BUNDLE)

rogue.so: $(SOURCES) src/rogue.gen
	$(CXX) $(FLAGS) $(FAST) -g -shared $(SOURCES) $(LVTK) $(LIBSRC) -Idsp -Isrc -o $@
	
rogue-gui.so: $(SOURCES_UI) src/rogue.gen src/gui/config.gen src/gui/rogue-gui.mcpp
	$(CXX) $(FLAGS) -g -shared $(SOURCES_UI) $(QT) $(LVTK) $(LVTK_UI) $(FFTW) -Idsp -Isrc -o $@	

src/rogue.gen: rogue.ttl
	ttl2c $^ src/rogue.gen
    
rogue.ttl:
	./confgen.py     
	
src/gui/config.gen:
	./confgen.py

src/gui/rogue-gui.mcpp:
	moc-qt4 src/gui/rogue-gui.cpp > src/gui/rogue-gui.mcpp

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

run:
	jalv.gtk http://www.github.com/timowest/rogue

run-qt:
	jalv.qt http://www.github.com/timowest/rogue

clean:
	rm -rf $(BUNDLE) *.so src/rogue.gen src/gui/config.gen src/gui/rogue-gui.mcpp rogue.ttl wavs *.out

gui: src/rogue.gen src/gui/config.gen src/gui/rogue-gui.mcpp	
	$(CXX) -g -std=c++11 src/gui/test.cpp $(QT) $(LVTK_UI) $(FFTW) -Idsp -Isrc -o qttest.out 
	
tests: src/rogue.gen
	$(CXX) -g -std=c++11 test/tests.cpp $(SNDFILE) $(FAST) -Idsp -Itest -o tests.out
	$(CXX) -g -std=c++11 test/voice_tests.cpp $(SNDFILE) $(LVTK) $(LIBSRC) -Idsp -Isrc -o voice_tests.out
	$(CXX) -g -std=c++11 test/fftw_tests.cpp $(FFTW) -o fftw_tests.out	
	mkdir -p wavs wavs/osc wavs/filter wavs/env wavs/lfo wavs/fx
	./tests.out	
	./voice_tests.out
	./fftw_tests.out
	
perf_tests: src/rogue.gen	
	$(CXX) -std=c++11 -fopenmp test/perf_tests.cpp $(FAST) -Idsp -Itest -o perf_tests.out
	./perf_tests.out
	
voice_perf_tests: src/rogue.gen
	$(CXX) -pg -std=c++11 -fopenmp test/voice_perf_tests.cpp -ftree-vectorizer-verbose=6 $(FAST) $(LVTK) $(LIBSRC) -Idsp -Isrc -o voice_perf_tests.out
	./voice_perf_tests.out
	gprof ./voice_perf_tests.out
