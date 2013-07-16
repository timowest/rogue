BUNDLE = lv2-rogue.lv2
INSTALL_DIR = /usr/local/lib/lv2

SOURCES = dsp/*.cpp src/*.cpp fx/*.cc
SOURCES_UI = dsp/lfo.cpp dsp/tables.cpp dsp/filter.cpp dsp/oscillator.cpp src/gui/rogue-gui.cpp
FLAGS = -fPIC -DPIC -std=c++11 
FAST = -Ofast -ffast-math
LVTK = `pkg-config --cflags --libs lvtk-plugin-1`
LVTK_UI = `pkg-config --cflags --libs lvtk-ui-1`
QT = `pkg-config --cflags --libs QtGui` 
SNDFILE = -lsndfile

$(BUNDLE): manifest.ttl rogue.ttl presets.ttl rogue.so rogue-gui.so presets styles
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp -r $^ $(BUNDLE)

rogue.so: $(SOURCES) src/rogue.gen
	$(CXX) $(FLAGS) $(FAST) -shared $(SOURCES) $(LVTK) -Idsp -Isrc -Ifx -Ifx/dsp -o $@
	
rogue-gui.so: $(SOURCES_UI) src/rogue.gen src/gui/config.gen
	$(CXX) $(FLAGS) -g -shared $(SOURCES_UI) $(QT) $(LVTK) $(LVTK_UI) -Idsp -Isrc -o $@	

src/rogue.gen: rogue.ttl
	ttl2c $^ src/rogue.gen
    
rogue.ttl:
	./confgen.py     

presets.ttl:
	mkdir -p presets
	./presetgen.py	
	
src/gui/config.gen:
	./confgen.py

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

run:
	jalv.qt http://www.github.com/timowest/rogue

clean:
	rm -rf $(BUNDLE) *.so src/rogue.gen src/gui/config.gen presets.ttl rogue.ttl wavs *.out presets/*

gui: src/rogue.gen src/gui/config.gen
	moc src/gui/rogue-gui.cpp > src/gui/rogue-gui.mcpp
	$(CXX) -g -std=c++11 src/gui/test.cpp $(QT) $(LVTK_UI) -Idsp -Isrc -o qttest.out 
	
tests: src/rogue.gen
	$(CXX) -g -std=c++11 test/dsp_tests.cpp $(SNDFILE) $(FAST) -Idsp -o dsp_tests.out
	$(CXX) -g -std=c++11 test/voice_tests.cpp $(SNDFILE) $(LVTK) -Idsp -Isrc -o voice_tests.out
	mkdir -p wavs
	./dsp_tests.out	
	./voice_tests.out