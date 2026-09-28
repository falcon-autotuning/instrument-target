#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

#include "instrument-target.h"
#include "instrument-target/instrument-target-export.h"

/**
 * @brief Push a InstrumentTarget into the Lua stack as userdata.
 *
 * @param L Lua state.
 * @param stack Pointer to an existing InstrumentTarget instance.
 * @param owned If non-zero, Lua takes ownership and will free it via GC.
 *
 * @note The userdata is pushed onto the Lua stack.
 *       The caller is responsible for assigning it (e.g. lua_setglobal).
 */
INSTRUMENT_TARGET_EXPORT void push_target(lua_State *L, InstrumentTarget *stack,
                                          int owned);

/**
 * @brief Lua module initializer.
 *
 * @param L Lua state.
 * @return Number of values returned to Lua (module table).
 *
 * @note Allows usage via require("instrument_target").
 */
INSTRUMENT_TARGET_EXPORT int luaopen_instrument_target(lua_State *L);

/**
 * @brief Push a InstrumentTarget into Lua and assign it to a global variable.
 *
 * @param L Lua state.
 * @param stack InstrumentTarget pointer.
 * @param owned If non-zero, Lua owns lifetime.
 * @param name Name of the global variable.
 */
INSTRUMENT_TARGET_EXPORT void push_target_global(lua_State *L,
                                                 InstrumentTarget *stack,
                                                 int owned, const char *name);

/**
 * @brief Register the InstrumentTarget module as a global Lua table.
 *
 * @param L Lua state.
 *
 * @note After calling this, Lua will have:
 *       instrument_target = { ... }
 */
INSTRUMENT_TARGET_EXPORT void register_instrument_target(lua_State *L);

/**
 * @brief Extract a InstrumentTarget pointer from a Lua userdata.
 *
 * @param L Lua state
 * @param index Stack index
 * @return InstrumentTarget* or NULL if not valid
 */
INSTRUMENT_TARGET_EXPORT
InstrumentTarget *lua_check_target(lua_State *L, int index);

#ifdef __cplusplus
}
#endif
