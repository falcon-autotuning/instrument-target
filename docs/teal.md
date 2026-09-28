# Using instrument-target with Teal

The Lua binding exposes `InstrumentTarget` objects as userdata with methods.  
These can be used in Teal via a type declaration file.

See the `teal/instrument-target.d.tl` for explicit types.

## Example

```lua
local target: InstrumentTarget= target-- injected from C

local name: string = stack:get_instrument_name()
local channel: int = stack:get_channel()
