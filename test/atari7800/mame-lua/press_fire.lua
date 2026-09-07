-- Scripted input injection for headless Atari 7800 test automation against
-- third_party/a7800 (see 7800port.md #19-#20). Presses/releases player 1's
-- fire button at a chosen frame, via MAME's own ioport_field:set_value --
-- not a keyboard/joystick simulation at the OS level, so it works under
-- `-video none` with no window or input focus at all.
--
-- Usage (all via environment variables, so this file never needs editing):
--   PRESS_AT_FRAME   frame to press fire on (default 120)
--   RELEASE_AT_FRAME frame to release it on (default PRESS_AT_FRAME + 10)
--   EXIT_AT_FRAME    frame to snapshot + exit on (default PRESS_AT_FRAME + 120)
--   SNAP_BEFORE      if "1", also snapshot the frame before pressing (default 1)
--
-- Example (see title-screen -> gameplay transition test in 7800port.md):
--   PRESS_AT_FRAME=120 EXIT_AT_FRAME=240 mame64 a7800 -cart astrowing.a78 \
--     -video none -window -nomax -seconds_to_run 5 \
--     -snapshot_directory out -autoboot_script press_fire.lua
--
-- Adjust the ioport tag/field below if targeting a different control-port
-- device -- this fork defaults joy1 to a Pro-Line-style two-button
-- joystick ("proline_joystick"); list actual tags/fields for the current
-- machine with `machine:ioport().ports[tag].fields`, keyed by field name.

local function getenv_num(name, default)
	local v = os.getenv(name)
	if v == nil then return default end
	return tonumber(v)
end

local press_at = getenv_num("PRESS_AT_FRAME", 120)
local release_at = getenv_num("RELEASE_AT_FRAME", press_at + 10)
local exit_at = getenv_num("EXIT_AT_FRAME", press_at + 120)
local snap_before = (os.getenv("SNAP_BEFORE") or "1") == "1"

local machine = manager:machine()
local port = machine:ioport().ports[":joy1:proline_joystick:JOY"]
local button = port.fields["P1 Button 1"]

local frame_count = 0
emu.register_frame(function()
	frame_count = frame_count + 1

	if snap_before and frame_count == press_at then
		manager:machine():video():snapshot()
	end
	if frame_count == press_at then
		print("press_fire.lua: pressing P1 Button 1 at frame " .. frame_count)
		button:set_value(1)
	end
	if frame_count == release_at then
		print("press_fire.lua: releasing P1 Button 1 at frame " .. frame_count)
		button:set_value(0)
	end
	if frame_count == exit_at then
		manager:machine():video():snapshot()
		manager:machine():exit()
	end
end)
