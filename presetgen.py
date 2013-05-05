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

def base(id, conf):
  m = {}
  for (k, v) in conf.items():
    m[id+"_"+k] = v
  return m

def osc(idx, conf):
  conf = conf.copy()
  conf.update({"on":1, "level":1.0})
  return base("osc"+str(idx), conf)

def dcf(idx, conf):
  conf = conf.copy()
  conf.update({"on":1, "level":1.0})
  return base("filter"+str(idx), conf)

def lfo(idx, conf):
  conf = conf.copy()
  conf.update({"on":1})
  return base("lfo"+str(idx), conf)

def env(idx, conf):
  conf = conf.copy()
  conf.update({"on":1})
  return base("env"+str(idx), conf)

def merge(*dicts):
  result = {}
  for d in dicts:
    result.update(d)
  return result

# TODO merge function

# Leads

# Basses

# FM

# Pianos

# Organs

# Brass

# Reed

# Guitars

# Strings

# Effects

def main():
  print merge(
          osc(1, {"type": 1}),
          dcf(1, {"type": 2}))

if __name__ == "__main__":
  main()
