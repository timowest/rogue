#!/usr/bin/python
# coding: utf-8

NS = """@prefix atom: <http://lv2plug.in/ns/ext/atom#> .
@prefix lv2: <http://lv2plug.in/ns/lv2core#> .
@prefix pset: <http://lv2plug.in/ns/ext/presets#> .
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
@prefix state: <http://lv2plug.in/ns/ext/state#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .
"""

def osc(idx, conf):
  id = "osc"+str(idx)
  m = {id+"_on": 1}
  m[id+"_level"] = 1.0
  for (k, v) in conf.items():
    m[id+"_"+k] = v
  return m

def dcf(id, conf):
  {}

def lfo(id, conf):
  {}

def env(id, conf):
  {}

# TODO merge function

# Leads

# Basses

# Brass

# Strings

# Effects

# FM

# Pianos

# Organs

def main():
  print osc(1, {"type": 1})

if __name__ == "__main__":
  main()
