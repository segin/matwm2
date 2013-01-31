/* vsb.js - Program logic.
 * This file is a part of Visual struct Builder
 * 
 * Copyright © 2013, Kirn Gill II <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

var typesList = [ 
	"char_arr",
	"int8_t",
	"uint8_t", 
	"int16_t",
	"uint16_t",
	"int32_t",
	"uint32_t",
	"int64_t",
	"uint64_t"
];

var typesName = [ 
	"char[]",
	"int8_t",
	"uint8_t", 
	"int16_t",
	"uint16_t",
	"int32_t",
	"uint32_t",
	"int64_t",
	"uint64_t"
];

function addElem() { 
	var elem = document.createElement("span");
	elem.className = "elem"; 
	var types = document.createElement("select");
	types.className = "etype";
	types.onchange = function() { 
		var len = types.parentElement.getElementsByClassName("elen");
		len[0].disabled = !(types.value == "char_arr");
	};
	for(var i = 0; i < typesList.length; i++) { 
		var item = new Option();
		item.value = typesList[i];
		item.text = typesName[i];
		types.options.add(item);
	}
	elem.appendChild(types);
	var lbl = document.createElement("span");
	lbl.textContent = "Length:";
	elem.appendChild(lbl);
	var classes = [ "elen", "ename" ];
	for(var i = 0; i < classes.length; i++) { 
		var input = document.createElement("input");
		input.className = classes[i];
		input.type = "text";
		elem.appendChild(input);
	}
	var btn = document.createElement("button");
	btn.textContent = "Remove";
	btn.onclick = function() {
		btn.parentElement.parentElement.removeChild(btn.parentElement); 
	};
	elem.appendChild(btn);
	elem.appendChild(document.createElement("br"));
	var list = window.document.getElementById("elems");
	list.appendChild(elem);
}
 
function resetBuilder() {
	document.getElementById("structname").value = "";
	document.getElementById("typedefname").value = "";
	document.getElementById("typedef").checked = false;
	var elemlist = document.getElementsByClassName("elem");
	for (var x = elemlist.length; x > 0; x--) { 
		document.getElementById("elems").removeChild(elemlist[x - 1]); 
	}
	addElem();
}

function typedefHide() {
	document.getElementById("tdnh").hidden = !document.getElementById("typedef").checked;
}

function generateCode() { 
	var struct = "", structName = document.getElementById("structname").value;
	var types = document.getElementsByClassName("etype");
	var lengths = document.getElementsByClassName("elen");
	var names = document.getElementsByClassName("ename");
	var typedef = document.getElementById("typedef");
	if (typedef.checked) {
		struct = "typedef ";
	}
	struct += "struct " + structName + " {\n";
	for(var i = 0; i < types.length; i++) { 
		var type, name;
		if (types[i].value == "char_arr") { 
			type = "char";
			name = names[i].value + "[" + lengths[i].value + "]"
		} else {
			type = types[i].value;
			name = names[i].value;
		}
		if (type.length < 8) {
			type += "\t";
		}
		struct += "\t" + type + "\t" + name + ";\n";
	}
	struct += "}";
	if (typedef.checked) {
		struct += " " + document.getElementById("typedefname").value;
	}
	struct += ";";
	document.getElementById("output").textContent = struct;
}