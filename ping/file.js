function template(locals) {
var buf = [];
var jade_mixins = {};
var jade_interp;
;var locals_for_with = (locals || {});(function (my_content) {
buf.push("<p>" + (jade.escape(null == (jade_interp = my_content) ? "" : jade_interp)) + "</p><h1>some Jade</h1>");}("my_content" in locals_for_with?locals_for_with.my_content:typeof my_content!=="undefined"?my_content:undefined));;return buf.join("");
}