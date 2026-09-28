#include <lauxlib.h>
#include <lua.h>

#include "instrument-target/instrument-target-lua.h"

/* =========================
   Lua wrapper struct
   ========================= */

typedef struct {
  InstrumentTarget *stack;
  int owned;
} lua_instrumenttarget;

/* =========================
   Helper
   ========================= */

static lua_instrumenttarget *check_instrumenttarget(lua_State *L) {
  return (lua_instrumenttarget *)luaL_checkudata(L, 1, "InstrumentTarget");
}

/* =========================
   Methods
   ========================= */

static int l_get_instrument_name(lua_State *L) {
  lua_instrumenttarget *cs = check_instrumenttarget(L);
  lua_pushstring(L, instrument_target_get_instrument_name(cs->stack));
  return 1;
}

static int l_get_channel_group(lua_State *L) {
  lua_instrumenttarget *cs = check_instrumenttarget(L);
  lua_pushstring(L, instrument_target_get_channel_group(cs->stack));
  return 1;
}

static int l_get_channel(lua_State *L) {
  lua_instrumenttarget *cs = check_instrumenttarget(L);
  lua_pushnumber(L, instrument_target_get_channel(cs->stack));
  return 1;
}

static int l_instrumenttarget_new(lua_State *L) {
  if (!lua_istable(L, 1)) {
    return luaL_error(L, "instrument_target.new expects a table");
  }

  const char *instrument = "";
  const char *group = "";
  const char *command = "";
  int channel = -1;

  /* instrument (required) */
  lua_getfield(L, 1, "instrument");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return luaL_error(L, "instrument is required");
  }
  instrument = luaL_checkstring(L, -1);
  lua_pop(L, 1);

  /* group (optional) */
  lua_getfield(L, 1, "group");
  if (!lua_isnil(L, -1)) {
    group = luaL_checkstring(L, -1);
  }
  lua_pop(L, 1);

  /* channel (optional) */
  lua_getfield(L, 1, "channel");
  if (!lua_isnil(L, -1)) {
    channel = (int)luaL_checkinteger(L, -1);
  }
  lua_pop(L, 1);

  InstrumentTarget *cs = instrument_target_create(instrument, group, channel);

  if (!cs) {
    return luaL_error(L, "Failed to create InstrumentTarget");
  }

  push_target(L, cs, 1);
  return 1;
}

static int l_instrumenttarget_clone(lua_State *L) {
  lua_instrumenttarget *cs = check_instrumenttarget(L);

  InstrumentTarget *copy =
      instrument_target_create(instrument_target_get_instrument_name(cs->stack),
                               instrument_target_get_channel_group(cs->stack),
                               instrument_target_get_channel(cs->stack));

  if (!copy) {
    return luaL_error(L, "Failed to clone InstrumentTarget");
  }

  push_target(L, copy, 1); // new Lua-owned object
  return 1;
}

static int l_instrumenttarget_tostring(lua_State *L) {
  lua_instrumenttarget *cs = check_instrumenttarget(L);

  lua_pushfstring(L, "InstrumentTarget(%s,%s,%d)",
                  instrument_target_get_instrument_name(cs->stack),
                  instrument_target_get_channel_group(cs->stack),
                  instrument_target_get_channel(cs->stack));

  return 1;
}
static int l_instrumenttarget_to_string(lua_State *L) {
  return l_instrumenttarget_tostring(L);
}

/* =========================
   GC
   ========================= */

static int l_instrumenttarget_gc(lua_State *L) {
  lua_instrumenttarget *cs = check_instrumenttarget(L);

  if (cs->owned && cs->stack) {
    instrument_target_free(cs->stack);
    cs->stack = NULL;
  }

  return 0;
}

/* =========================
   Methods table
   ========================= */

static const luaL_Reg instrumenttarget_methods[] = {
    {"get_instrument_name", l_get_instrument_name},
    {"get_channel_group", l_get_channel_group},
    {"get_channel", l_get_channel},
    {"clone", l_instrumenttarget_clone},
    {"to_string", l_instrumenttarget_to_string},
    {NULL, NULL}};

/* =========================
   Public API
   ========================= */

void push_target(lua_State *L, InstrumentTarget *stack, int owned) {
  lua_instrumenttarget *cs =
      (lua_instrumenttarget *)lua_newuserdata(L, sizeof(lua_instrumenttarget));

  cs->stack = stack;
  cs->owned = owned;

  luaL_getmetatable(L, "InstrumentTarget");
  lua_setmetatable(L, -2);
}

void push_target_global(lua_State *L, InstrumentTarget *stack, int owned,
                        const char *name) {
  push_target(L, stack, owned);
  lua_setglobal(L, name);
}

void register_instrument_target(lua_State *L) {
  luaopen_instrument_target(L);
  lua_setglobal(L, "instrument_target");
}

/* =========================
   Module Init
   ========================= */

static const luaL_Reg module_funcs[] = {{"new", l_instrumenttarget_new},
                                        {NULL, NULL}};

int luaopen_instrument_target(lua_State *L) {
  luaL_newmetatable(L, "InstrumentTarget");

  lua_pushcfunction(L, l_instrumenttarget_tostring);
  lua_setfield(L, -2, "__tostring");

  lua_pushcfunction(L, l_instrumenttarget_gc);
  lua_setfield(L, -2, "__gc");

  lua_newtable(L);
  luaL_setfuncs(L, instrumenttarget_methods, 0);
  lua_setfield(L, -2, "__index");

  lua_pop(L, 1);

  lua_newtable(L);
  luaL_setfuncs(L, module_funcs, 0);

  return 1;
}

InstrumentTarget *lua_check_target(lua_State *L, int index) {
  lua_instrumenttarget *cs =
      (lua_instrumenttarget *)luaL_testudata(L, index, "InstrumentTarget");

  if (!cs || !cs->stack) {
    return NULL;
  }

  return cs->stack;
}
