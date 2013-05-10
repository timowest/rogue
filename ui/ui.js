$(document).ready(function() {
  var browserTmpl = Handlebars.compile($("#browser-tmpl").html());
  var masterTmpl = Handlebars.compile($("#master-tmpl").html());
  var oscTmpl = Handlebars.compile($("#osc-tmpl").html());
  var filterTmpl = Handlebars.compile($("#filter-tmpl").html());
  var envTmpl = Handlebars.compile($("#env-tmpl").html());
  var modulationTmpl = Handlebars.compile($("#modulation-tmpl").html());
  var lfoTmpl = Handlebars.compile($("#lfo-tmpl").html());
  
  $(".browser").html(browserTmpl({}));
  $(".global").html(masterTmpl({}));
  $(".osc").html(oscTmpl({}));
  $(".filter").html(filterTmpl({}));
  $(".envs").html(envTmpl({}));
  $(".modulation").html(modulationTmpl({}));
  $(".lfos").html(lfoTmpl({}));
});