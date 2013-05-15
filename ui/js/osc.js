var PI = 3.14159265359;
var TWO_PI = 2 * PI;

function sin2(x) { 
    return Math.sin(TWO_PI * x);     
}

// virtual analog

function va_tri(x) {
  return fabs(gb(x));
}
function va_saw(x) {
  return gb(x);
}
function va_saw_tri(x) {
  return x;
}
function va_pulse(x, w) {
  return gb(gpulse(x, w));
}

// phase shaping / distortion

function ps_sin(x) {
  return sin2(x);
}

// electronic

function el_saw(x) {
  return gb(x);
}
function el_tri(x) {
  return fabs(gb(x));
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