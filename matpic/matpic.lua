-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
--   MPASM                                                                   --
--   Small, inefficient and fully featured PIC assembler in lua.             --
--   Copryright (c) 2012 Mattis Michel.                                      --
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

--[[
  SYNOPSIS
    mpasm [<input file>] [<output file>]
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
        define <identifier> [<value>]
          Tells the preprocessor to replace any occurence of <identifier>
          with <value> (which can be anything, whitespace included). Also
          it can be used without any value for ifdef and ifndef.
        ifdef <identifier>
          Starts a block of code that is removed by the preprocessor unless
          <identifier> is defined with a prior define directive. These blocks
          can be nested.
        ifndef <identifier>
          Same as ifdef but removes the block when the identifier has been
          defined, instead of the other way around.
        endif
          Ends either of the blocks spoken of above.
    Assembler directives description follows:
        include <filename>
          Threat the file specified by <filename> as if it was in place of
          this directive.
        org <address>
          Tells the assembler following bytecode will go to the adress
          specified by <address>, the argument may be any integer expression.
        data <word>, ...
          Insert the data specified by <word> directly into the output.
          More than one argument amounts to the same as using the directive
          multiple times.
    Arguments to any assembler directive or instruction can be integer
    expressions using the following operators: +, -, *, /, %, |, &, ^, <<, >>,
    ~, !.
    Add, substract, multiply, divide, modula/remainder, bitwise OR, AND, XOR,
    shift left, shift right, NOT and NOT respectively. There are parsed
    strictly left to right. And bracelets (only normal round ones) can be used
    to group operations.
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

  TODO/BUGS
    - New integer parser instead of rather limited tonumber(). Currently
      integer parsing does not work as advertised (no binary notation etc).
    - Builtin disassembler would be nice.
    - Support 12bit and 16bit chips, only 14bit supported for now.
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
	["sublw"] = { op=0x3D00, a="k8", m=0x0100 },
	["xorlw"] = { op=0x3A00, a="k8" }
}

function comp14(v)
	-- lua, Y U NO HAVE BITWISE OPERATORS
	if  v.ins.a == "df" and table.getn(v.args) == 2 then
		v.args[1] = v.args[1] % 2
		v.args[2] = v.args[2] % 0x7F
		v.ins.op = v.ins.op + (v.args[2] * 0x80) + v.args[1]
	elseif v.ins.a == "f" and table.getn(v.args) == 1 then
		v.args[1] = v.args[1] % 0x7F
		v.ins.op = v.ins.op + v.args[1]
	elseif v.ins.a == "bf" and table.getn(v.args) == 2 then
		v.args[1] = v.args[1] % 4
		v.args[2] = v.args[2] % 0x7F
	elseif v.ins.a == "k8" and table.getn(v.args) == 1 then
		v.args[1] = v.args[1] % 0xFF
		v.ins.op = v.ins.op + v.args[1]
	elseif v.ins.a == "k11" and table.getn(v.args) == 1 then
		v.args[1] = v.args[1] % 0x7FF
		v.ins.op = v.ins.op + v.args[1]
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

------------------------
-- bunch of functions --
------------------------

function pp(lines)
	local d, v
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
			if d and not defines[d] then return "", x + 1 end
			_, _, d = string.find(ln, "^%s+ifndef%s+([%a_][%a%d_]*)$")
			if d and defines[d] then return "", x + 1 end
			if string.find(ln, "^%s+endif$") then return "", 0 end
			return ln, 0
		end
	end

	for i, ln in ipairs(lines) do
		line = i

		-- strip any comments and trailing whitespace,
		--   maek everything lowercase
		ln = string.gsub(ln, "%s*;.*$", "")
		ln = string.gsub(ln, "%s*$", "")
		ln = string.lower(ln)

		-- substitute any macros
		ln = string.gsub(ln, "([%a_][%a%d_]*)", function (w)
			if defines[w] then return defines[w] end
		end)

		-- handle ifdef, ifndef and endif
		ln, x = ckifdef(ln, x)

		-- handle defines
		if x == 0 then
			_, _, d, v = string.find(ln, "^%s+define%s+([%a_][%a%d_]*)%s+(.*)$")
			if d == nil then
				_, _, d = string.find(ln, "^%s+define%s+([%a_][%a%d_]*)$")
				v = ""
			end
			if d then
				defines[d] = v
				ln = ""
			end
		end

		lines[i] = ln
	end

	return lines
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
			if v.ins then
				file = v.file
				line = v.line
				arch.comp(v)
			end
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
		c = len + (address % 0x100) + math.floor(address / 0x100)
		s = string.format(":%02X%04X00", len * 2, address)
		for i, v in ipairs(l) do
			c = c + v
			s = s .. string.format("%04X", v)
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
			table.insert(l, v.ins.op)
			len = len + 1
			if len == outwidth then endline() end
		else -- data directive
            for i, d in ipairs(v.args) do
				table.insert(l, v.ins.args)
				-- in case we hit max line width
				table.remove(v.args, i)
				len = len + 1
				if len == outwidth then endline() end
			end
		end
	end
	endline()
	table.insert(r, ":0000001FF")
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
	local e, e2, c0, args = { }
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
				errexit("org wants exactly 1 argument")
			end
			assemblefile(args[1])
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
	_, e, c0 = string.find(ln, "^%s+([%a%d_]+)")
	if c0 == nil then errexit("syntax error") end
	ln = string.sub(ln, e + 1, ln.length)
	if ln ~= "" then
		ln, e = string.gsub(ln, "^%s+", "")
		if e == 0 then errexit("syntax error") end
		args = {}
		string.gsub(ln, "([^,]+)", function (a) table.insert(args, a) end)
	end
	if directive[c0] then
		directive[c0](args)
	elseif arch.instr[c0] then
		table.insert(data, { ["ins"] = arch.instr[c0], ["args"] = args,
		             ["line"] = line, ["file"] = file })
		address = address + 1
	else errexit(c0 .. "is neither a directive or an instruction") end
end

function calc(l, o, r)
	if     o == "+" then return l + r
	elseif o == "-" then return l - r
	elseif o == "*" then return l * r
	elseif o == "/" then return math.floor(l / r)
	elseif o == "%" then return math.floor(l % r)
	elseif o == "|" then
		local b = 0
		local e = 0
		while l > 0 or l > 0 do
			if (l % 2) == 1 or (r % 2) == 1 then
				e = e + (2 ^ b)
			end
			r = math.floor(r / 2)
			l = math.floor(l / 2)
			b = b + 1
		end
		return e
	elseif o == "&" then
		local b = 0
		local e = 0
		while l > 0 or l > 0 do
			if (l % 2) == 1 and (r % 2) == 1 then
				e = e + (2 ^ b)
			end
			r = math.floor(r / 2)
			l = math.floor(l / 2)
			b = b + 1
		end
		return e
	elseif o == "^" then
		local b = 0
		local e = 0
		while l > 0 or l > 0 do
			if ((l % 2) == 1 or (r % 2) == 1) and (l % 2) ~= (r % 2) then
				e = e + (2 ^ b)
			end
			r = math.floor(r / 2)
			l = math.floor(l / 2)
			b = b + 1
		end
		return e
	elseif o == "<<" then return l * (2 ^ r)
	elseif o == ">>" then return math.floor(l / (2 ^ r))
	else errexit("syntax error") end
end

function parseint(str)
	local r = 0
	r = tonumber(str) -- to be replaced
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
	str = string.gsub(str,
	                  "^%s*([%xx_]+)%s*([%+-%*/%%|&%^<>]+)%s*(%d[%xx_]*)",
	                  function (l, o, r)
		l = parseint(l)
		r = parseint(r)
		return calc(l, o, r)
	end)
	r = parseint(str)
	if r == nil then errexit("syntax error") end
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
	io.stderr:write("line " .. line .. msg .. "\n")
	os.exit(1)
end

----------------------
-- invoke the magic --
----------------------

assemblefile(arg[1])
writeoutput(arg[2])