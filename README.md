# instrument-target

`instrument-target` is a small C99 library for representing and transporting instrument target metadata. It provides a simple, opaque data structure with fixed-size storage and supports serialization for use across process or language boundaries. Optional Lua bindings allow read-only access to the same data model from embedded scripting environments.

The library is designed for use in instrumentation systems, telemetry pipelines, and cross-language integrations where predictable memory layout and minimal allocation overhead are required.

***

## Overview

A `instrument-target` represents three pieces of metadata:

* Instrument name
* Channel group
* Channel

Each field is stored as a fixed-size string, allowing for a compact and stable in-memory representation. This enables efficient serialization and avoids dynamic allocation within the object itself. The Channel is an int.

***

## C API

### Creation and lifetime

```c
InstrumentTarget *instrument_target_create(
    const char *instrument_name,
    const char *channel_group,
    int channel,
    const char *command);

void instrument_target_free(InstrumentTarget *target);
```

The object is immutable after creation and must be released with `instrument_target_free`.

***

## Lua Bindings

Lua bindings are optional and controlled by the `BUILD_LUA` build option. When enabled, `InstrumentTarget` objects can be created and used directly from Lua as userdata with read-only access to their fields.

***

### Creating InstrumentTargets

InstrumentTargets are created using a table-based constructor:

```lua
local target= instrument_target.new{
  instrument = "instrument",
  group      = "group",   -- optional
  channel    = 1          -- optional
}
```

#### Required fields

* `instrument`

#### Optional fields

* `group` (defaults to `""`)
* `channel` (defaults to `-1`)

***

### Available methods

```lua
target:get_instrument_name()
target:get_channel_group()
target:get_channel()

target:clone()
target:to_string()
```

#### Notes

* `clone()` returns a new independent `InstrumentTarget`
* `to_string()` returns a formatted string representation
* `tostring(target)` is also supported via Lua’s `__tostring` metamethod

***

### Example

```lua
local target1 = instrument_call_target.new{
  instrument = "i",
  group = "g",
  channel = 1,
}

local target2 = target1:clone()

print(target1:get_command())    -- "cmd"
print(target2:to_string())      -- InstrumentTarget(i,g,1,cmd)
print(target1 ~= target2)        -- true
```

***

### Minimal example

Only required fields need to be provided:

```lua
local target = instrument_target.new{
  instrument = "i",
}

print(target:get_channel())      -- -1
print(target:get_channel_group())-- ""
```

***

## Teal Support

This library can be used from Teal (typed Lua) via the provided
type definitions.

See `docs/teal.md` for details.

***

## Building

The project uses CMake and a Makefile wrapper. Dependencies are managed via vcpkg.

### Configure and build

```bash
make build
```

Build configuration is controlled through CMake presets. The default presets enable tests and can optionally enable Lua bindings.

### Enabling Lua

Lua support is enabled via:

```bash
cmake --preset <preset> -DBUILD_LUA=ON
```

Alternatively, presets may already define this flag.

***

## Running Tests

Tests are built and run through the same workflow:

```bash
make test
```

This executes both the core C tests and, when enabled, the Lua binding tests.

***

## Design Considerations

The library uses fixed-size storage to ensure predictable memory usage and straightforward serialization. This makes it well suited for systems where ABI stability and minimal overhead are prioritized. The API enforces immutability, allowing safe concurrent read access as long as object lifetime is externally managed.

Lua bindings are intentionally limited to read-only operations to preserve consistency with the C data model and prevent unexpected mutations across language boundaries.

***

## License

See [LICENSE](LICENSE) for details.
