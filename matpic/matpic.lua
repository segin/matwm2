-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
--   MatPIC                                                                  --
--   Small, inefficient and fully featured PIC assembler in lua.             --
--   Copryright (c) 2012 Mattis Michel.                                      --
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

--[[
  SYNOPSIS
    matpic [<input file>] [<output file>]
      If output file is not specified, output will be written to stdout.
      If input file is not specified, stdin will be used.

  SYNTAX
    Comments can be started with the ';' character, and end at the next
    newline.
    Identifiers can start with either underscore or an aphabetic character,
    and be composted of aphanumeric characters and underscores.
    An identifier at the start of a line (before any whitespace) is considered
    a label, and can then be used as argument to any function or directive,
    except preprocessor directives.
    Anything that is preceded by whitespace is considered either an
    instruction or a directive, and can be followed by arguments separated
    with commas.
    There are 2 kinds of directives: preprocessor directives and assembler
    directives.
    The following preprocessor directives exist (all of which are executed
    before anything else:
        define <identifier>, [<value>]
          Tells the preprocessor to replace any occurence of <identifier>
          with <value> (which can be anything, whitespace included). Also
          it can be used without any value for ifdef and ifndef.
		enum <expression>, <identifier>, [<...>]
          Start a series of defines which will refer to consecutive numbers,
          starting from <expression>.
        ifdef <identifier>
          Starts a block of code that is removed by the preprocessor unless
          <identifier> is defined with a prior define directive. These blocks
          can be nested.
        ifndef <identifier>
          Same as ifdef but removes the block when the identifier has been
          defined, instead of the other way around.
        endif
          Ends either of the blocks spoken of above.
        include <filename>
          Threat the file specified by <filename> as if it was in place of
          this directive.
		message <message>
		  Print a message to stderr.
		error <message>
		  Print an error message to stderr and exit.
    Assembler directives description follows:
        org <address>
          Tells the assembler following bytecode will go to the adress
          specified by <address>, the argument may be any integer expression.
        data <word>, [<...>]
          Insert the data specified by <word> directly into the output.
          More than one argument amounts to the same as using the directive
          multiple times.
    Arguments to any assembler directive or instruction can be integer
    expressions using the following operators: +, -, *, /, %, |, &, ^, <<, >>,
    ~, !.
    Add, substract, multiply, divide, modula/remainder, bitwise OR, AND, XOR,
    shift left, shift right, NOT and NOT respectively. They are parsed
    strictly left to right. Bracelets (only normal round ones) can be used to
    group operations.
    Integers can be preceded with "0x" for hexadecimal, "0b" for binary,
    "0o" or "0" for octal, and "0d" or nothing for decimal. They can also be
    visually divided with underscores, which will be ignored.
    Nothing is case sensitive.

  LEGENDA
    [something]
      Square bracelets indicate something optional.
    <something>
      Triangle bracelets indicate something you should replace with whatever
      you prefer.
    [<...>]
      Optionally more arguments of the same type as the last one can be given.

  TODO/BUGS
	- Perhaps something like banksel would be nice?
    - Builtin disassembler would be nice.
    - Fix dumb manual.
    - Support 12bit and 16bit chips, only 14bit supported for now.
    - MPASM compatibility?
]]--

------------------------------
-- architecture definitions --
------------------------------

local instr14 = {
	["addwf"] = { op=0x0700, a="df" },
	["andwf"] = { op=0x0500, a="df" },
	["clrf"] = { op=0x0180, a="f" },
	["clrw"] = { op=0x0100, m=0x007F },
	["comf"] = { op=0x0900, a="df" },
	["decf"] = { op=0x0300, a="df" },
	["decfsz"] = { op=0x0B00, a="df" },
	["incf"] = { op=0x0A00, a="df" },
	["incfsz"] = { op=0x0F00, a="df" },
	["iorwf"] = { op=0x0400, a="df" },
	["movf"] = { op=0x0800, a="df" },
	["movwf"] = { op=0x0080, a="f" },
	["nop"] = { op=0x0000, m=0x0060 },
	["rlf"] = { op=0x0D00, a="df" },
	["rrf"] = { op=0x0C00, a="df" },
	["subwf"] = { op=0x0200, a="df" },
	["swapf"] = { op=0x0E00, a="df" },
	["xorwf"] = { op=0x0600, a="df" },
	["bcf"] = { op=0x1000, a="bf" },
	["bsf"] = { op=0x1400, a="bf" },
	["btfsc"] = { op=0x1800, a="bf" },
	["btfss"] = { op=0x1C00, a="bf" },
	["addlw"] = { op=0x3E00, a="k8", m=0x0100 },
	["andlw"] = { op=0x3900, a="k8" },
	["call"] = { op=0x2000, a="k11" },
	["clrwdt"] = { op=0x0064 },
	["goto"] = { op=0x2800, a="k11" },
	["iorlw"] = { op=0x3800, a="k11" },
	["movlw"] = { op=0x3000, a="k8", m=0x0300 },
	["retfie"] = { op=0x0009 },
	["retlw"] = { op=0x3400, a="k8", m=0x0300 },
	["return"] = { op=0x0008 },
	["sleep"] = { op=0x0063 },
	["sublw"] = { op=0x3C00, a="k8", m=0x0100 },
	["xorlw"] = { op=0x3A00, a="k8" },
	["option"] = { op=0x0062 },
	["tris"] = { op=0x0060, a="t" },
}

function comp14(v)
	-- lua, Y U NO HAVE BITWISE OPERATORS
	if  v.ins.a == "df" and table.getn(v.args) == 2 then
		v.args[1] = v.args[1] % 0x80
		v.args[2] = v.args[2] % 2
		v.op = v.ins.op + (v.args[2] * 0x80) + v.args[1]
	elseif v.ins.a == "f" and table.getn(v.args) == 1 then
		v.args[1] = v.args[1] % 0x80
		v.op = v.ins.op + v.args[1]
	elseif v.ins.a == "bf" and table.getn(v.args) == 2 then
		v.args[1] = v.args[1] % 0x80
		v.args[2] = v.args[2] % 8
		v.op = v.ins.op + (v.args[2] * 0x80) + v.args[1]
	elseif v.ins.a == "k8" and table.getn(v.args) == 1 then
		v.args[1] = v.args[1] % 0x100
		v.op = v.ins.op + v.args[1]
	elseif v.ins.a == "k11" and table.getn(v.args) == 1 then
		v.args[1] = v.args[1] % 0x800
		v.op = v.ins.op + v.args[1]
	elseif v.ins.a == "t" and table.getn(v.args) == 1 then
		v.args[1] = v.args[1] % 8
		v.op = v.ins.op + v.args[1]
	elseif v.ins.a == nil and v.args == nil then
		v.op = v.ins.op
	else
		errexit("invalid number of arguments to instruction")
	end
end

local pic14 = {
	["instr"] = instr14,
	["comp"] = comp14,
}

----------------------------
-- a few global variables --
----------------------------

local arch = pic14  -- atm only pic14 supported (16xxxx and some 12xxxx chips)
local outwidth = 8  -- number of 16bit words per line in ihex output files
local file
local line
local address = 0
local data = {}
local labels = {}
local defines = {}
local includes = {}

------------------------
-- bunch of functions --
------------------------

function splitln(ln)
	local r = {}

	-- strip any comments and trailing whitespace,
	ln = ln:gsub("%s*;.*$", "")
	ln = ln:gsub("%s*$", "")

	-- divide everything in a hopefully useful fashion
	r.ln = ln
	ln = ln:gsub("^([^%s]+)", function (l) r.label = l:lower() return "" end)
	ln = ln:gsub("^%s+([^%s]+)", function (i) r.ins = i:lower() return "" end)
	r.args = ln

	return r
end

function splitargs(args)
	local r, c
	args:gsub("^%s+(.+)", function(a)
		r = {}
		repeat
			a, c = a:gsub("^([^,]+)", function (a)
				table.insert(r, a)
				return ""
			end)
			if c == 0 then errexit("empty argument") end
			ln, c = ln:gsub("^,%s*", "")
		until c == 0
	end)
	return r
end

function pp(lines)
	local d, v, e, fn
	local x = 0

	function ckifdef(ln, x)
		if x > 0 then
			if string.find(ln, "^%s+endif$") then
				return "", x - 1
			elseif string.find(ln, "^%s+ifdef%s+[%a_][%a%d_]*$") then
				return "", x + 1
			elseif string.find(ln, "^%s+ifndef%s+[%a_][%a%d_]*$") then
				return "", x + 1
			end
			return "", x
		else
			_, _, d = string.find(ln, "^%s+ifdef%s+([%a_][%a%d_]*)$")
			if d then
				if not defines[d] then return "", x + 1 end
				return "", x
			end
			_, _, d = string.find(ln, "^%s+ifndef%s+([%a_][%a%d_]*)$")
			if d then
				if defines[d] then return "", x + 1 end
				return "", x
			end
			if string.find(ln, "^%s+endif$") then return "", 0 end
			return ln, 0
		end
	end

	for i, ln in ipairs(lines) do
		line = i

		-- strip any comments and trailing whitespace,
		-- maek everything lowercase
		ln = ln:gsub("%s*;.*$", "")
		ln = ln:gsub("%s*$", "")
		ln = ln:lower()

		-- handle ifdef, ifndef and endif
		ln, x = ckifdef(ln, x)

		if x == 0 then
			-- handle message and error directives
			_, _, d = ln:find("^%s+message%s+(.*)$")
			if d then
				io.stderr:write(d.."\n")
				ln = ""
			end
			_, _, d = ln:find("^%s+error%s+(.*)$")
			if d then errexit(d) end

			-- handle defines
			_, _, d, v = ln:find("^%s+define%s+([%a_][%a%d_]*)%s+(.*)$")
			if d == nil then
				_, _, d = ln:find("^%s+define%s+([%a_][%a%d_]*)$")
				v = ""
			end
			if d then
				v = v:gsub("([%a_][%a%d_]*)", function (w)
					if defines[w] then return defines[w] end
				end)
				defines[d] = v
				ln = ""
			end

			-- substitute any macros
			ln = ln:gsub("([%a_][%a%d_]*)", function (w)
				if defines[w] then return defines[w] end
			end)

			-- handle include
			_, _, d = ln:find("^[%a%d_]*%s+include%s+(.*)$")
			if d then
				fn = file
				ppfile(d)
				file = fn
			end

			-- handle enum
			_, e, d = ln:find("^%s+enum%s+(.*)$")
			if d then
				local args = {}
				local pos
				string.gsub(d, "([^,]+)", function (a) table.insert(args, a) end)
				if table.getn(args) < 2 then
					errexit("too few arguments to directive 'enum'")
				end
				pos = parsemath(args[1])
				table.remove(args, 1)
				for i, v in ipairs(args) do
					v = v:gsub("^%s*(.*)%s*$", "%1")
					if not v:find("^[%a_][%a%d_]*$") then
						errexit("syntax error 1")
					end
					defines[v] = pos
					pos = pos + 1
				end
				ln = ""
			end
		end

		lines[i] = ln
	end

	return lines
end

function ppfile(fn)
	local lines = {}
	local infile = io.stdin
	if fn then infile = io.open(fn, "r") end
	if infile == nil then
		io.stderr:write("failed to open file " .. fn .. "\n")
		os.exit(1)
	end
	file = fn
	for ln in infile:lines() do
		table.insert(lines, ln)
	end
	infile:close()
	pp(lines)
	includes[fn] = lines
end

function assemble(lines)
	-- preprocessor
	lines = pp(lines)

	-- get all labels, opcodes, etc
	for i, ln in ipairs(lines) do
		line = i
		parseln(ln)
	end

	-- parse arguments, put everything together
	for i,v in ipairs(data) do
		if v.args then
			for i,a in ipairs(v.args) do
				file = v.file
				line = v.line
				v.args[i] = resolve(a)
			end
		end
		if v.ins then
			file = v.file
			line = v.line
			arch.comp(v)
		end
	end
end

function assemblefile(fn)
	local lines = {}
	local infile = io.stdin
	if fn then infile = io.open(fn, "r") end
	if infile == nil then
		io.stderr:write("failed to open file " .. fn .. "\n")
		os.exit(1)
	end
	file = fn
	for ln in infile:lines() do
		table.insert(lines, ln)
	end
	infile:close()
	return assemble(lines)
end

function getoutput()
	local r = {}
	local l = {}
	local len = 0
	local address = 0

	function endline()
		local c, s
		if len == 0 then return end
		c = (len * 2) + (address % 0x100) + math.floor(address / 0x100)
		s = string.format(":%02X%04X00", len * 2, address)
		for i, v in ipairs(l) do
			c = c + (v % 0x100) + math.floor(v / 0x100)
			s = s .. string.format("%02X%02X", v % 0x100, math.floor(v / 0x100))
		end
		s = s .. string.format("%02X", (0x100 - (c % 0x100)) % 0x100)
		table.insert(r, s)
		address = address + (len * 2)
		len = 0
		l = {}
	end

	for i,v in ipairs(data) do
		if v.address then
			endline()
			address = v.address * 2
		elseif v.ins then
			table.insert(l, v.op)
			len = len + 1
			if len == outwidth then endline() end
		else -- data directive
            while v.args[1] do
				table.insert(l, v.args[1])
				table.remove(v.args, 1)
				len = len + 1
				if len == outwidth then endline() end
			end
		end
	end
	endline()
	table.insert(r, ":00000001FF")
	return r
end

function writeoutput(fn)
	local lines = getoutput()
	local outfile = io.stdout
	if fn then outfile = io.open(fn, "w") end
	if outfile == nil then
		io.stderr:write("failed to open file " .. fn .. "\n")
		os.exit(1)
	end
	for i, v in ipairs(lines) do
		outfile:write(v .. "\n")
	end
	outfile:close()
end

function reset()
	file = nil
	line = nil
	address = 0
	data = {}
	labels = {}
	defines = {}
end

function parseln(ln)
	local e, e2, c0, args
	local directive = {
		["org"] = function (args)
			if args and table.getn(args) ~= 1 then
				errexit("org wants exactly 1 argument")
			end
			address = parsemath(args[1])
			table.insert(data, { ["address"] = address, ["line"] = line,
			             ["file"] = file })
		end,
		["data"] = function (args)
			if args and table.getn(args) < 1 then
				errexit("data wants one or more arguments")
			end
			table.insert(data, { ["args"] = args, ["line"] = line,
			             ["file"] = file })
			address = address + table.getn(args)
		end,
		["include"] = function(args)
			local ft = file
			if args and table.getn(args) ~= 1 then
				errexit("include wants exactly 1 argument")
			end
			file = args[1]
			assemble(includes[args[1]])
			file = ft
		end,
	}

	-- find label, if any
	_, e, c0 = string.find(ln, "^([%a_][%a%d_]*)")
	if c0 then
		ln = string.sub(ln, e + 1, ln.length)
		labels[c0] = { ["value"] = address, ["line"] = line, ["file"] = file }
	end

	-- look for instruction or directive
	if ln == "" then return end
	_, e, c0 = string.find(ln, "^%s+([%a_][%a%d_]*)")
	if c0 == nil then errexit("syntax error 2") end
	ln = string.sub(ln, e + 1, ln.length)
	if ln ~= "" then
		ln, e = string.gsub(ln, "^%s+", "")
		if e == 0 then errexit("syntax error 3") end
		args = {}
		string.gsub(ln, "([^,]+)", function (a) table.insert(args, a) end)
	end
	if directive[c0] then
		directive[c0](args)
	elseif arch.instr[c0] then
		table.insert(data, { ["ins"] = arch.instr[c0], ["args"] = args,
		             ["line"] = line, ["file"] = file })
		address = address + 1
	else errexit("syntax error 4") end
end

function bit_or(l, r)
	local b = 0
	local e = 0
	while l > 0 or r > 0 do
		if (l % 2) == 1 or (r % 2) == 1 then
			e = e + (2 ^ b)
		end
		r = math.floor(r / 2)
		l = math.floor(l / 2)
		b = b + 1
	end
	return e
end

function bit_and(l, r)
	local b = 0
	local e = 0
	while l > 0 or r > 0 do
		if (l % 2) == 1 and (r % 2) == 1 then
			e = e + (2 ^ b)
		end
		r = math.floor(r / 2)
		l = math.floor(l / 2)
		b = b + 1
	end
	return e
end

function bit_xor(l, r)
	local b = 0
	local e = 0
	while l > 0 or r > 0 do
		if ((l % 2) == 1 or (r % 2) == 1) and (l % 2) ~= (r % 2) then
			e = e + (2 ^ b)
		end
		r = math.floor(r / 2)
		l = math.floor(l / 2)
		b = b + 1
	end
	return e
end

function bit_shl(l, r) return l * (2 ^ r) end

function bit_shr(l, r) return math.floor(l / (2 ^ r)) end

function calc(l, o, r)
	if     o == "+" then return l + r
	elseif o == "-" then return l - r
	elseif o == "*" then return l * r
	elseif o == "/" then return math.floor(l / r)
	elseif o == "%" then return l % r
	elseif o == "|" then return bit_or(l, r)
	elseif o == "&" then return bit_and(l, r)
	elseif o == "^" then return bit_xor(l, r)
	elseif o == "<<" then return bit_shl(l, r)
	elseif o == ">>" then return bit_shr(l, r)
	else errexit("syntax error 5") end
end

function parseint(str)
	local n
	local r = 0
	local b = 10
	local nums = {
		["0"] = 0,  ["1"] = 1,  ["2"] = 2,  ["3"] = 3,
		["4"] = 4,  ["5"] = 5,  ["6"] = 6,  ["7"] = 7,
		["8"] = 8,  ["9"] = 9,  ["a"] = 10, ["b"] = 11,
		["c"] = 12, ["d"] = 13, ["e"] = 14, ["f"] = 15,
	}
	str = string.gsub(str, "^%s*(.*)%s*$", "%1")
	if string.find(str, "^0") then
		local d = 3
		if string.find(str, "^0x") then b = 16
		elseif string.find(str, "^0b") then b = 2
		elseif string.find(str, "^0d") then b = 10
		elseif string.find(str, "^0o") then b = 8
		else
			b = 8
			d = 2
		end
		str = string.sub(str, d)
	end
	for c in string.gmatch(str, ".") do
		if c ~= "_" then
			n = nums[c]
			if n == nil then errexit("invalid numeric constant "..str) end
			if not (n < b) then errexit("invalid numeric constant "..str) end
			r = (r * b) + n
		end
	end
	return r
end

function parsemath(str)
	local r
	str = string.gsub(str, "(%b())", function (x)
		return parsemath(string.sub(x, 2, -2))
	end)
	str = str.gsub(str, "[!~]%s*(%d[%xx_]*)", function (x)
		local r = 0
		local b = 0
		while b < 16 do
			r = r + (1 - ((x % 2) * (2 ^ b)))
			x = math.floor(x / 2)
			b = b + 1
		end
		return x
	end)
	repeat
		str, r = string.gsub(str,
		                  "^%s*([%xx_]+)%s*([%+-%*/%%|&%^<>]+)%s*(%d[%xx_]*)",
		                  function (l, o, r)
			l = parseint(l)
			r = parseint(r)
			return calc(l, o, r)
		end)
	until r == 0
	r = parseint(str)
	if r == nil then errexit("syntax error 6") end
	return r
end

function resolve(str)
	str = string.gsub(str, "([%a_][%a%d_]*)", function (w)
		local l = labels[w]
		if l then return l.value end
		return w
	end)
	return parsemath(str)
end

function errexit(msg)
	if file then io.stderr:write(file .. ": ") end
	io.stderr:write("line " .. line .. ": " .. msg .. "\n")
	os.exit(1)
end

----------------------
-- invoke the magic --
----------------------

assemblefile(arg[1])
writeoutput(arg[2])
