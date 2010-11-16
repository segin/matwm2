--[[ 

matserve-lua.lua: A simple webserver in standard Lua.

Copyright (c) 2010 Mattis Michel <sic_zer0@hotmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

]]

local htdocs = "./htdocs/"
local hostname = "127.0.0.1"
local port = 80

local socket = require("socket")
local ls = socket.tcp()

function respond(s, a, b, c, d)
	local rf = "HTTP/1.1 %s\nContent-Type: %s\nContent-Length: %i\n\n%s"
	s:send(string.format(rf, a, b, string.len(c), c))
end

function hc(s)
		local l = s:receive("*l");
		local c = {}
		local p = {}
		string.gsub(l, "[^ ]+", function(x) table.insert(c, x) end)
		if c[1] == "GET" then
			string.gsub(c[2], "[^/]+", function(x)
				if x == ".." then
					table.remove(p)
				else
					table.insert(p, x)
				end
			end)
			local f = io.open(htdocs..table.concat(p), "r")
			if f ~= nil then
				respond(s, "200 OK", "text/html", f:read("*all"))
			else
				respond(s, "404 Not found", "text/html", "Error 404: Not found")
			end
		else
			respond(s, "400 Bad request", "text/html", "Error 400: Bad request")
		end
		s:shutdown()
end

ls:bind(hostname, port)
ls:listen()

while true do
	local tr = coroutine.create(hc)
	coroutine.resume(tr, ls:accept())
end
