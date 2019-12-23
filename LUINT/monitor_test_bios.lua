R"( -- BIOS STARTS HERE "

-- This is the default computer BIOS used in every Processing Unit created.
-- The BIOS of a State Machine is a small program that is executed on startup. Normally, it defines some critical
-- system functions and executes an operating system main file found in a filesystem. However, you can also program
-- it to do other things.


-- main() is called on machine startup. TODO: remove main and execute all BIOS contents, not just this function
function main()
	print("BIOS started")

	local function pull()
		repeat
			coroutine.yield()
		until latest_event[1] ~= "tick"

		return table.unpack(latest_event)
	end

	local screen = nil
	local uid = nil
	for k, v in pairs(computer.connections) do
		if v.name == "PTM Monitor" then
			screen = v
			uid = k
			break
		end
	end

	if screen == nil then
		print("Couldn't find any screen.")
		return
	end

	local function write(string, x, y)
		for i = 1, #string do
			screen.set(string:sub(i, i), x + i - 1, y)
		end
	end
	screen.write = write

	screen.write("Testing monitor, 1 2 3...", 1, 1)
	screen.write("Screen UID: " .. uid, 1, 2)

	local i = 1
	local dir = 1
	screen.write("Latest event not tick: <nil>", 1, 4)
	repeat
		screen.fill(" ", 1, 3, 40, 1)
		screen.write("Hello world!", i, 3)
		i = i + dir
		if i > 20 then
			dir = -1
		elseif i == 1 then
			dir = 1
		end
		coroutine.yield()
		if latest_event[1] ~= "tick" then
			screen.write("Latest event not tick: "..latest_event[1], 1, 4)
		end
	until false
end

-- BIOS ENDS HERE
)"