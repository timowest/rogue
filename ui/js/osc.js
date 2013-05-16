var PI = 3.14159265359;
var M_PI = PI;
var TWO_PI = 2 * PI;

function tanh (arg) {
  return (Math.exp(arg) - Math.exp(-arg)) / (Math.exp(arg) + Math.exp(-arg));
}


function sin2(x) { 
    return Math.sin(TWO_PI * x);     
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

// phase shaping

function pd(x, w) {
  if (x < w) {
    return 0.5 * x / w;    
  } else {
    return 0.5 + 0.5 * (x-w) / (1-w); 
  }  
}

function ps_sin(x, w) {
  var x2 = pd(x, w);  
  return sin2(x2);
}

function ps_sin_saw(x, w) {
  var x2 = pd(x, w);  
  return sin2(x2 + 0.25);
}

function ps_sin_fm(x, w) {  
  return sin2(x + (w-0.5) * sin2(x));
}

function ps_sin_half(x, w) {
  var x2 = pd(x, w);  
  return gb(sin2(0.5 * x2)); 
}

// electronic

function el_saw(x) {
  return gb(x);
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

function el_slope(x, w) {
  return gb(gvslope(x, w));
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