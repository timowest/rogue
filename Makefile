BUNDLE = lv2-rogue.lv2
INSTALL_DIR = /usr/local/lib/lv2

SOURCES = dsp/*.cpp src/*.cpp
FLAGS = -fPIC -DPIC -std=c++11 -O $(WARNINGS) -Idsp -Isrc
WARNINGS = #-g -Wall -pedantic

$(BUNDLE): manifest.ttl rogue.ttl rogue.so presets
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp -r $^ $(BUNDLE)

rogue.so: $(SOURCES) src/rogue.peg
	$(CXX) $(FLAGS) -shared $(SOURCES) `pkg-config --cflags --libs daps-plugin-1.0` -o $@

src/rogue.peg: rogue.ttl
	ttl2c $^ src/rogue.peg
    
rogue.ttl:
	./confgen.py > rogue.ttl    

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE) rogue.so src/rogue.peg rogue.ttl	