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
