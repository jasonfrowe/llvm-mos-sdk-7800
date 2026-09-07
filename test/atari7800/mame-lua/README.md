# MAME Lua test scripts

Reusable Lua scripts for driving `third_party/a7800`'s `mame64` headlessly --
input injection, memory/register introspection, and the like. These are a
different tool from the Libretro-core smoke tests elsewhere in `test/`
(`../smoke.c`, `../CMakeLists.txt`): they exercise this SDK's own owned test
emulator directly, via `-autoboot_script`, and are meant for interactive or
scripted investigation (screenshots, watchpoints, input scripting) rather
than pass/fail CI assertions.

See `7800port.md` (search for "third_party/a7800") for how this emulator
fork came to be owned/buildable here, and for the debugger-console fixes
(`-debugger none` processing `-debugscript` at all, and mirroring its
output to stdout) that make real watchpoints usable headlessly.

## `press_fire.lua`

Presses/releases player 1's fire button at a chosen frame via MAME's own
`ioport_field:set_value` -- works under `-video none` with no window or
input focus. Used to script the title-screen "press start" -> gameplay
transition. Configured entirely via environment variables (see the file's
own header comment); example:

```sh
PRESS_AT_FRAME=120 EXIT_AT_FRAME=240 \
  third_party/a7800/mame64 a7800 -cart path/to/game.a78 \
  -video none -window -nomax -seconds_to_run 5 \
  -snapshot_directory out -skip_gameinfo \
  -autoboot_script test/atari7800/mame-lua/press_fire.lua
```

Snapshots land in `out/a7800/0000.png` (just before the press) and
`out/a7800/0001.png` (at `EXIT_AT_FRAME`).

If a game uses a different control-port device than this fork's default
Pro-Line-style two-button joystick, list the actual ioport tags/fields for
the current machine from another script with:

```lua
for tag, port in pairs(machine:ioport().ports) do
  for name, field in pairs(port.fields) do
    print(tag, name)
  end
end
```
