BUNDLE = lv2-rogue.lv2
INSTALL_DIR = /usr/local/lib/lv2

SOURCES = dsp/*.cpp src/*.cpp
FLAGS = -fPIC -DPIC -std=c++11 -O -Idsp -Isrc
LVTK = `pkg-config --cflags --libs lvtk-plugin-1`
GTKMM = `pkg-config --cflags --libs gtkmm-2.4`

$(BUNDLE): manifest.ttl rogue.ttl rogue.so presets
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp -r $^ $(BUNDLE)

rogue.so: $(SOURCES) src/rogue.gen
	$(CXX) $(FLAGS) -g -shared $(SOURCES) $(LVTK) -o $@

src/rogue.gen: rogue.ttl
	ttl2c $^ src/rogue.gen
    
rogue.ttl:
	./confgen.py     
	
src/gui/config.gen:
	./confgen.py

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

run:
	jalv.gtk −g http://www.github.com/timowest/rogue

clean:
	rm -rf $(BUNDLE) rogue.so src/rogue.peg rogue.ttl *.out
	
knobtest:	
	$(CXX) src/gui/knob-test.cpp $(GTKMM) -Isrc -o knobtest.out		