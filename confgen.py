#!/usr/bin/python
# coding: utf-8

PREFIX = """@prefix atom:  <http://lv2plug.in/ns/ext/atom#> .
@prefix doap: <http://usefulinc.com/ns/doap#>.
@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix ll:   <http://ll-plugins.nongnu.org/lv2/namespace#>.
@prefix lv2:  <http://lv2plug.in/ns/lv2core#>.
@prefix pg:   <http://ll-plugins.nongnu.org/lv2/ext/portgroups#>.
@prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>.
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.
@prefix ui:    <http://lv2plug.in/ns/extensions/ui#>.
@prefix urid:  <http://lv2plug.in/ns/ext/urid#>.

<http://www.github.com/timowest/rogue/out> a pg:StereoGroup.

<http://www.github.com/timowest/rogue/ui>
  a ui:Qt4UI ;
  ui:binary <rogue-gui.so>;
  lv2:requiredFeature urid:map .

<http://www.github.com/timowest/rogue>
  a lv2:Plugin, lv2:InstrumentPlugin;
  lv2:binary <rogue.so>;
  doap:name "rogue";
  doap:maintainer [
    a foaf:Person;
    foaf:name "Timo Westkämper";
    foaf:homepage <http://www.github.com/timowest>
  ];
  doap:license <http://usefulinc.com/doap/licenses/gpl>;
  ll:pegName "p";
  ui:ui <http://www.github.com/timowest/rogue/ui> ;

  lv2:port [
    a lv2:InputPort, atom:AtomPort;
    lv2:index 0 ;
    atom:bufferType atom:Sequence;
    atom:supports <http://lv2plug.in/ns/ext/midi#MidiEvent>,
                  <http://lv2plug.in/ns/ext/patch#Message> ;    
    lv2:symbol "control" ;
    lv2:name "Control"
  ] , [
    a lv2:AudioPort, lv2:OutputPort;
    lv2:index 1;
    lv2:symbol "left";
    lv2:name "Left";
    pg:membership [
      pg:group <http://www.github.com/timowest/rogue/out>;
      pg:role pg:leftChannel;
    ];
  ] , [
    a lv2:AudioPort, lv2:OutputPort;
    lv2:index 2;
    lv2:symbol "right";
    lv2:name "Right";
    pg:membership [
      pg:group <http://www.github.com/timowest/rogue/out>;
      pg:role pg:rightChannel;
    ];
  ]"""

PREFIX_GUI = """#ifndef ANALOGUE_META
#define ANALOGUE_META

typedef struct {
    const char* symbol;
    float min;
    float max;
    float default_value;
    float step;
} port_meta_t;

static const port_meta_t p_port_meta[] = {
    {"control", 0, 0, 0, 1},
    {"left", 0, 0, 0, 1},
    {"right", 0, 0, 0, 1},"""

def ttl_control(idx, symbol, name, min, max, default):
    if (min == 0 and max == 1 and isinstance(max, int) and default == 0):
        return """ , [
    a lv2:ControlPort, lv2:InputPort;
    lv2:index %s;
    lv2:symbol "%s";
    lv2:name "%s";
    lv2:minimum %s;
    lv2:default 0;
    lv2:portProperty lv2:toggled
  ]""" % (idx, symbol, name, min)
    elif isinstance(max, int):
        return """ , [
    a lv2:ControlPort, lv2:InputPort;
    lv2:index %s;
    lv2:symbol "%s";
    lv2:name "%s";
    lv2:minimum %s;
    lv2:maximum %s;
    lv2:default %s;
    lv2:portProperty lv2:integer
  ]""" % (idx, symbol, name, min, max, default)
    else:
        return """ , [
    a lv2:ControlPort, lv2:InputPort;
    lv2:index %s;
    lv2:symbol "%s";
    lv2:name "%s";
    lv2:minimum %s;
    lv2:maximum %s;
    lv2:default %s
  ]""" % (idx, symbol, name, min, max, default)

def port_meta(symbol, min, max, default, step):
    return '    {"%s", %s, %s, %s, %s},' % (symbol, min, max, default, step)

def controls(ttl, gui, idx, type, count, controls):
    for i in range(count):
        prefix = type+str(i+1)+"_"
        for c in controls:
            # override default
            c3 = 1 if (i == 0 and c[0] == "on") else c[3]
            ttl.append(ttl_control(idx, prefix+c[0], c[0], c[1], c[2], c3))
            gui.append(port_meta(prefix+c[0], c[1], c[2], c3, c[4]))
            idx += 1
    return idx

def main():

    #        suffix        min max default 

    oscs = [["on"         , 0, 1, 0, 1], # toggled
            ["type"       , 0, 9, 0, 1],
            ["inv"        , 0, 1, 0, 1], # toggled
            ["free"       , 0, 1, 0, 1], # toggled
            ["tracking"   , 0, 1, 1, 1], # toggled
            ["ratio"      , 0, 16.0, 1, 0.1],
            ["coarse"     , -48, 48, 0, 1.0],
            ["fine"       , -1.0, 1.0, 0, 0.01],
            ["start"      , 0, 1.0, 0.0, 0.01],
            ["width"      , 0, 1.0, 0.5, 0.01],
            ["level_a"    , 0, 1.0, 0, 0.01],
            ["level_b"    , 0, 1.0, 0, 0.01],
            ["level"      , 0, 1.0, 0, 0.01],
            
            ["input"      , 0, 2, 0, 1],
            ["pm"         , 0, 1.0, 0, 0.01],
            ["sync"       , 0, 1,   0, 1], # toggled
            ["out_mod"    , 0, 3, 0, 1]]    
            
    dcfs = [["on"         , 0, 1, 0, 1], # toggled
            ["type"       , 0, 11, 0, 1],
            ["source"     , 0, 2, 0, 1],
            ["freq"       , 0, 10000.0, 440.0, 10.0],
            ["q"          , 0, 1.0, 0, 0.01],
            ["distortion" , 0, 1.0, 0, 0.01], 
            ["level"      , 0, 1.0, 0, 0.01],
            ["pan"        , 0, 1.0, 0.5, 0.01], 
            
            ["key_to_f"   , 0, 1.0, 0, 0.01],
            ["vel_to_f"   , 0, 1.0, 0, 0.01]]
  
    lfos = [["on"         , 0, 1, 0, 1], # toggled
            ["type"       , 0, 4, 0, 1],
            ["inv"        , 0, 1, 0, 1], # toggled
            ["reset_type" , 0, 2, 0, 1],
            ["freq"       , 0, 10.0, 10.0, 0.1],
            ["start"      , 0, 1.0, 0.0, 0.01],
            ["width"      , 0, 1.0, 0.5, 0.01],
            ["humanize"   , 0, 1.0, 0, 0.01]]
  
    envs = [["on"         , 0, 1, 0, 1], # toggled
            ["pre_delay"  , 0, 5.0, 0, 0.01],
            ["attack"     , 0, 5.0, 0.1, 0.01],
            ["hold"       , 0, 5.0, 0, 0.01],
            ["decay"      , 0, 5.0, 0.5, 0.01],
            ["sustain"    , 0, 1.0, 0.8, 0.01],
            ["release"    , 0, 5.0, 0.5, 0.01],
            ["curve"      , 0, 1.0, 0.5, 0.01],
            ["retrigger"  , 0, 1, 0, 1]] # toggled
                        
    mods = [["src"        , 0, 17, 0, 1],
            ["target"     , 0, 43, 0, 1],
            ["amount"     , -1.0, 1.0, 0, 0.01]]        

    idx = 3

    gui = []
    gui.append(PREFIX_GUI)

    ttl = []
    ttl.append(PREFIX)
    # oscs
    idx = controls(ttl, gui, idx, "osc", 4, oscs)
    # dcfs
    idx = controls(ttl, gui, idx, "filter", 2, dcfs)
    # lfos
    idx = controls(ttl, gui, idx, "lfo", 3, lfos)
    # envs
    idx = controls(ttl, gui, idx, "env", 5, envs)
    # mods
    idx = controls(ttl, gui, idx, "mod", 20, mods)

    globals = [["bus_a_level", 0, 1.0, 0, 0.01],
               ["bus_a_pan",   0, 1.0, 0.5, 0.01],
               ["bus_b_level", 0, 1.0, 0, 0.01],
               ["bus_b_pan",   0, 1.0, 0.5, 0.01],
               ["volume",      0, 1.0, 0.5, 0.01],

               ["glide_time",  0, 5.0, 0, 0.1],
               ["bend_range",  0, 12.0, 0, 0.1],
               # TODO poly and mono modes

               ["chorus_on",     0, 1, 0, 1],
               ["chorus_t",      2.5, 25, 8.1, 0.1],
               ["chorus_width",  0.5, 10, 1, 0.1],
               ["chorus_rate",   0, 1, 0.25, 0.01],
               ["chorus_blend",  0, 1, 0.25, 0.01],
               ["chorus_feedforward", 0, 1, 1, 0.1],
               ["chorus_feedback", 0, 1, 0.5, 0.1],

               ["phaser_on",     0, 1, 0, 1],
               ["phaser_rate",   0, 1, 0.25, 0.01],
               ["phaser_depth",  0, 1, 1, 0.01],
               ["phaser_spread", 0, 1, 0.5, 0.01],
               ["phaser_resonance", 0, 1, 0, 0.01],

               ["delay_on",      0, 1, 0, 1],
               ["delay_bpm",     30, 164, 97, 0.1],
               ["delay_divider", 2, 4, 3, 1],
               ["delay_feedback",0, 1, 0.75, 0.01],
               ["delay_dry",     0, 1, 0.5, 0.01],
               ["delay_blend",   0, 1, 1, 0.01],
               ["delay_tune",    415, 467, 440, 0.1],

               ["reverb_on",     0, 1, 0, 1],
               ["reverb_bandwidth", 0, 1, 0.75, 0.01],
               ["reverb_tail",   0, 1, 0.5, 0.01],
               ["reverb_damping",0, 1, 0.25, 0.01],
               ["reverb_blend",  0, 1, 0.25, 0.01]]


    for c in globals:
        ttl.append(ttl_control(idx, c[0], c[0], c[1], c[2], c[3]))
        gui.append(port_meta(c[0], c[1], c[2], c[3], c[4]))
        idx += 1

    ttl.append(".")

    gui.append("};")
    gui.append("#endif")

    cppFile = open("rogue.ttl", "w")
    cppFile.write("\n".join(ttl))
    cppFile.close()

    guiFile = open("src/gui/config.gen", "w")
    guiFile.write("\n".join(gui))
    guiFile.close()
    

if __name__ == "__main__":
    main()

