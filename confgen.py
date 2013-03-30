#!/usr/bin/python
# coding: utf-8

PREFIX = """@prefix atom:  <http://lv2plug.in/ns/ext/atom#> .
@prefix doap: <http://usefulinc.com/ns/doap#>.
@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix ll:   <http://ll-plugins.nongnu.org/lv2/namespace#>.
@prefix lv2:  <http://lv2plug.in/ns/lv2core#>.
@prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>.
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.
@prefix pg:   <http://ll-plugins.nongnu.org/lv2/ext/portgroups#>.

<http://www.github.com/timowest/rogue/out> a pg:StereoGroup.

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

def control(idx, symbol, name, min, max, default):
    if (min == 0 and max == 1 and isinstance(max, int) and default == 0):
        print """ , [
    a lv2:ControlPort, lv2:InputPort;
    lv2:index %s;
    lv2:symbol "%s";
    lv2:name "%s";
    lv2:minimum %s;
    lv2:portProperty lv2:toggled
  ]""" % (idx, symbol, name, min)
    else:
        print """ , [
    a lv2:ControlPort, lv2:InputPort;
    lv2:index %s;
    lv2:symbol "%s";
    lv2:name "%s";
    lv2:minimum %s;
    lv2:maximum %s;
    lv2:default %s;
  ]""" % (idx, symbol, name, min, max, default)

    return idx + 1

def controls(idx, type, count, controls):
    for i in range(count):
        prefix = type+str(i+1)+"_"
        for c in controls:
            idx = control(idx, prefix+c[0], c[0], c[1], c[2], c[3])
    return idx

def main():

    oscs = [["on"         , 0, 1, 0], # toggled
            ["type"       , 0, 9, 0],
            ["inv"        , 0, 1, 0], # toggled
            ["free"       , 0, 1, 0], # toggled
            ["tracking"   , 0, 1, 0], # toggled
            ["ratio"      , 0, 16.0, 1],
            ["coarse"     , -48.0, 48.0, 0],
            ["fine"       , -1.0, 1.0, 0],
            ["param1"     , 0, 1.0, 0],
            ["param2"     , 0, 1.0, 0],
            ["level_a"    , 0, 1.0, 0],
            ["level_b"    , 0, 1.0, 0],
            ["volume"     , 0, 1.0, 0],
            ["vel_to_vol" , 0, 1.0, 0 ]]
    
    dcfs = [["on"         , 0, 1, 0], # toggled
            ["type"       , 0, 11, 0],
            ["source"     , 0, 2, 0],
            ["freq"       , 0, 20000.0, 440.0],
            ["q"          , 0, 1.0, 0],
            ["distortion" , 0, 1.0, 0], 
            ["level"      , 0, 1.0, 0],
            ["pan"        , -1.0, 1.0, 0], # ?
            ["key_to_f"   , 0, 1.0, 0],
            ["vel_to_f"   , 0, 1.0, 0]]
  
    lfos = [["on"         , 0, 1, 0], # toggled
            ["type"       , 0, 5.0, 0],
            ["reset_type" , 0, 2, 0],
            ["freq"       , 0, 10.0, 1000.0],
            ["symmetry"   , 0, 1.0, 0.5],
            ["attack"     , 0, 5.0, 0],
            ["decay"      , 0, 5.0, 0],
            ["humanize"   , 0, 1.0, 0],
            ["key_to_f"   , 0, 1.0, 0]]
  
    envs = [["on"         , 0, 1, 0], # toggled
            ["pre_delay"  , 0, 5.0, 0],
            ["attack"     , 0, 5.0, 0],
            ["hold"       , 0, 5.0, 0],
            ["decay"      , 0, 5.0, 0],
            ["sustain"    , 0, 1.0, 1.0],
            ["release"    , 0, 5.0, 0],
            ["retrigger"  , 0, 1, 0], # toggled
            ["vel_to_vol" , 0, 1.0, 0],
            ["key_to_speed" , 0, 1.0, 0],
            ["vel_to_speed" , 0, 1.0, 0]]

    idx = 3

    print PREFIX
    # oscs
    idx = controls(idx, "osc", 4, oscs)
    # dcfs
    idx = controls(idx, "filter", 2, dcfs)
    # lfos
    idx = controls(idx, "lfo", 3, lfos)
    # envs
    idx = controls(idx, "env", 5, envs)

    globals = [["bus_a_level", 0, 1.0, 0],
               ["bus_a_pan",   0, 1.0, 0.5],
               ["bus_b_level", 0, 1.0, 0],
               ["bus_b_pan",   0, 1.0, 0.5],
               ["volume",      0, 1.0, 0.5],

               ["glide_time",  0, 5.0, 0],
               ["bend_range",  0, 12.0, 0]]

    for c in globals:
        idx = control(idx, c[0], c[0], c[1], c[2], c[3])

    print "."
    

if __name__ == "__main__":
    main()

