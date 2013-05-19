var PI = 3.14159265359;
var M_PI = PI;
var TWO_PI = 2 * PI;

function tanh (arg) {
  return (Math.exp(arg) - Math.exp(-arg)) / (Math.exp(arg) + Math.exp(-arg));
}


function sin2(x) { 
    return Math.sin(TWO_PI * x);     
}

function cos2(x) {
  return Math.cos(TWO_PI * x);
}

// virtual analog

function mod_saw(x, P) {
  var m = M_PI - 2.0 * M_PI * P;
  if (x < P) {
      return m * (x / P);
  } else {
      return m * (1.0 - x) / (1.0 - P);
  }
}

function va_saw(phase) {
  var freq = 44.0;
  var P = 0.9924 - 0.00002151 * freq;
  return Math.sin(2.0 * M_PI * phase + mod_saw(phase, P) - 0.5 * M_PI);
}

// TODO
function va_tri(x) {
  return 0;
}

//TODO
function va_saw_tri(x) {
  return 0;
}

//TODO
function va_pulse(x, w) {
  return 0;
}

// phase distortion
// based on https://github.com/smbolton/whysynth
// based on http://www.amazona.de/workshop-casio-czvz-und-die-grundlagen-der-phase-distortion-synthesis/3/

function pd(x, w) {
  if (x < w) {
    return 0.5 * x / w;    
  } else {
    return 0.5 + 0.5 * (x-w) / (1.0 - w); 
  }  
}

function pd_saw(x, w) {
  var mod = 0.5 - w * 0.5;
  return cos2(pd(x, mod));
}

function pd_square(x, w) {
  var mod = 0.5 - w * 0.5;
  var x2 = 0.0;
  if (x < mod) {
    x2 = x * 0.5 / mod;
  } else if (x < 0.5) {
    x2 = 0.5;
  } else if (x < 0.5 + mod) {
    x2 = (x - 0.5) * 0.5 / mod + 0.5;
  } else {
    x2 = 1.0;
  }
  return cos2(x2);
}

function pd_pulse(x, w) {
  var mod = 1.0 - w;
  var x2 = 0;
  if (x < mod) {
    x2 = x / mod;
  } else {
    x2 = 1.0;
  }
  return cos2(x2);
}

function pd_double_sine(x, w) {
  var mod = 1.0 - w;
  var x2 = 0;
  if (x < 0.5) {
    x2 = 2.0 * x;
  } else {
    x2 = 1.0 - (x - 0.5) / (0.5 * mod);
    if (x2 < 0) x2 = 0;
  }
  return cos2(x2);
}

function pd_saw_pulse(x, w) {
  var mod = 1.0 - w;
  var x2 = 0;
  if (x < 0.5) {
    x2 = x;
  } else {
    x2 = 0.5 - (x - 0.5) / mod;
    if (x2 < 0) x2 = 0;
  }
  return cos2(x2);
}

function pd_res1(x, w) {
  var mod = Math.exp(w * 6.0); // FIXME
  var x2 = x * mod;
  var window = 1.0 - x;
  return 0.5 - window * (0.5 - cos2(x2));
}

function pd_res2(x, w) {
  var mod = Math.exp(w * 6.0); // FIXME
  var x2 = x * mod;
  var window = x < 0.5 ? 2.0 * x : 2.0 * (1.0 - x);
  return 0.5 - window * (0.5 - cos2(x2));
}

function pd_res3(x, w) {
  var mod = Math.exp(w * 6.0); // FIXME
  var x2 = x * mod;
  var window = x < 0.5 ? 1.0 : 2.0 * (1.0 - x);
  return 0.5 - window * (0.5 - cos2(x2));
}


// preserve
function pd_sin_half(x, w) {
  var mod = 0.5 - w * 0.5;
  return gb(sin2(0.5 * pd(x, mod))); 
}

// electronic

function el_saw(x) {
  return gb(x);
}

function el_double_saw(x, w) {
  var x2 = pd(x, w);
  if (x < w) {
    return (x2 * 4.0) - 1.0;
  } else {
    return (x2-0.5) * 4.0 - 1.0;
  }
}

function tri(x, w) {
  if (x < w) {
    return x / w;
  } else {
    return 1 - (x-w) / (1-w);
  }
}

function el_tri(x, w) {
  return gb(tri(x, w));
}

function el_tri2(x, w) {
  var p = tri(x, w);
  var p2 = p * Math.sqrt(p) + (1-p) * p;
  return gb(p2);
}

function el_tri3(x, w) {
  var p = tri(x, w);
  var p2 = 0;
  if (x < w) {
    p2 = Math.sqrt(p);    
  } else {    
    p2 = p * p * p;
  }
  return gb(p2);
}

function el_pulse(x, w) {
  return gb(gpulse(x, w));
}

function el_pulse_saw(x, w) {
  var x2 = pd(x, w);
  if (x < w) {
    return x2 * 2.0;
  } else {
    return (x2 - 0.5) * -2.0;
  }
}

function el_slope(x, w) {
  return gb(gvslope(x, w));
}

function el_alpha1(x, w) {
  var pw = gpulse((2.0 * x) % 1.0, w);
  var saw = el_saw(x);
  return gb(pw * (saw + 1.0) - 1.0)
}

function el_alpha2(x, w) {
  var pw = gpulse((4.0 * x) % 1.0, w);
  var saw = el_saw(x);
  return gb(pw * (saw + 1.0) - 1.0)
}

// additive synthesis

function as_saw(x, w) {
  var y = 0;
  for (var i = 1.0; i < (20.0 * w); i++) {
    y += sin2(i * x) * 1.0/i;
  }
  return -0.55 * y;
}

function as_square(x, w) {
  var y = 0;
  for (var i = 1.0; i < (40.0 * w); i += 2) {
    y += sin2(i * x) * 1.0/i;
  }
  return y;
}

// noise

function no_white(x) {
  return gb(Math.random());
}

function update_plot() {
  var width = parseFloat($("#width").val());
  console.log(width);
  
  $(".plot").each(function() {
    var fn = window[$(this).attr("data-fn")];    
    var arr = [];
    for (var p = 0.0; p < 1.5; p += 0.01) {
      arr.push([p, fn(fmod(p,1.0), width)])
    }
    $.plot(this, [arr]);
  });
}
 
$(document).ready(function() {
//  var d2 = [[0, 3], [4, 8], [8, 5], [9, 13]];
//  var d3 = [[0, 12], [7, 12], null, [7, 2.5], [12, 2.5]];
//  $.plot("#plot", [ d2, d3 ]);
  
  update_plot();
  $("#width").change(update_plot);
});