#include <cmocka.h>
#include <stdarg.h>
#include <stddef.h>

#include <lauxlib.h>
#include <lualib.h>

#include "instrument-target/instrument-target-lua.h"
#include "instrument-target/instrument-target.h"

/* =========================
   Helpers
   ========================= */

static lua_State *create_lua(void) {
  lua_State *L = luaL_newstate();
  assert_non_null(L);

  luaL_openlibs(L);

  luaopen_instrument_target(L);
  lua_pop(L, 1);

  return L;
}

/* =========================
   Tests
   ========================= */

static void test_lua_getters_basic(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create("i", "g", 1);

  push_target(L, cs, 0);
  lua_setglobal(L, "target");

  assert_int_equal(luaL_dostring(L,
                                 "assert(target:get_instrument_name() == 'i')\n"
                                 "assert(target:get_channel_group() == 'g')\n"
                                 "assert(target:get_channel() == 1)\n"),
                   LUA_OK);

  instrument_target_free(cs);
  lua_close(L);
}

static void test_push_target_global(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create("g1", "g2", 3);

  push_target_global(L, cs, 0, "global_target");

  assert_int_equal(luaL_dostring(L, "assert(global_target:get_channel() == 3)"),
                   LUA_OK);

  instrument_target_free(cs);
  lua_close(L);
}

static void test_register_module(void **state) {
  (void)state;

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  register_instrument_target(L);

  assert_int_equal(luaL_dostring(L, "assert(instrument_target ~= nil)"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_owned_gc(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create("x", "y", 4);

  push_target(L, cs, 1);
  lua_setglobal(L, "target");

  luaL_dostring(L, "target=nil; collectgarbage()");

  lua_close(L);
}

static void test_lua_not_owned_gc(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create("x", "y", 4);

  push_target(L, cs, 0);
  lua_setglobal(L, "target");

  luaL_dostring(L, "target=nil; collectgarbage()");

  assert_non_null(cs);

  instrument_target_free(cs);
  lua_close(L);
}

static void test_lua_null_safety(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create(NULL, NULL, -1);

  push_target(L, cs, 0);
  lua_setglobal(L, "target");

  assert_int_equal(luaL_dostring(L,
                                 "assert(target:get_instrument_name() == '')\n"
                                 "assert(target:get_channel_group() == '')\n"
                                 "assert(target:get_channel() == -1)\n"),
                   LUA_OK);

  instrument_target_free(cs);
  lua_close(L);
}

static void test_lua_to_string_method(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create("i", "g", 2);

  push_target(L, cs, 0);
  lua_setglobal(L, "target");

  assert_int_equal(luaL_dostring(L, "local s = target:to_string()\n"
                                    "assert(type(s) == 'string')\n"
                                    "assert(s:match('InstrumentTarget%('))\n"
                                    "assert(s:match('i'))\n"
                                    "assert(s:match('g'))\n"
                                    "assert(s:match('2'))\n"),
                   LUA_OK);

  instrument_target_free(cs);
  lua_close(L);
}

static void test_lua_tostring_metamethod(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create("i2", "g2", 5);

  push_target(L, cs, 0);
  lua_setglobal(L, "target");

  assert_int_equal(luaL_dostring(L, "local s = tostring(target)\n"
                                    "assert(type(s) == 'string')\n"
                                    "assert(s:match('InstrumentTarget%('))\n"
                                    "assert(s:match('i2'))\n"
                                    "assert(s:match('g2'))\n"
                                    "assert(s:match('5'))\n"),
                   LUA_OK);

  instrument_target_free(cs);
  lua_close(L);
}

static void test_lua_constructor_basic(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_target(L);

  assert_int_equal(luaL_dostring(L, "local cs = instrument_target.new{\n"
                                    "  instrument = 'a',\n"
                                    "  group = 'b',\n"
                                    "  channel = 7,\n"
                                    "}\n"
                                    "assert(cs:get_instrument_name() == 'a')\n"
                                    "assert(cs:get_channel_group() == 'b')\n"
                                    "assert(cs:get_channel() == 7)\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_constructor_defaults(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_target(L);

  assert_int_equal(luaL_dostring(L, "local cs = instrument_target.new{\n"
                                    "  instrument = 'x',\n"
                                    "}\n"
                                    "assert(cs:get_instrument_name() == 'x')\n"
                                    "assert(cs:get_channel_group() == '')\n"
                                    "assert(cs:get_channel() == -1)\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_clone_basic(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_target(L);

  assert_int_equal(luaL_dostring(L, "local cs1 = instrument_target.new{\n"
                                    "  instrument = 'x',\n"
                                    "  group = 'y',\n"
                                    "  channel = 9,\n"
                                    "}\n"
                                    "local cs2 = cs1:clone()\n"
                                    "assert(cs2:get_instrument_name() == 'x')\n"
                                    "assert(cs2:get_channel_group() == 'y')\n"
                                    "assert(cs2:get_channel() == 9)\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_clone_independence(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_target(L);

  assert_int_equal(luaL_dostring(L, "local cs1 = instrument_target.new{\n"
                                    "  instrument = 'a',\n"
                                    "  group = 'b',\n"
                                    "  channel = 1,\n"
                                    "}\n"
                                    "local cs2 = cs1:clone()\n"
                                    "assert(cs1 ~= cs2)\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_clone_gc_owned(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_target(L);

  assert_int_equal(luaL_dostring(L, "local cs1 = instrument_target.new{\n"
                                    "  instrument = 'x',\n"
                                    "  group = 'y',\n"
                                    "  channel = 3,\n"
                                    "}\n"
                                    "local cs2 = cs1:clone()\n"
                                    "cs1 = nil\n"
                                    "cs2 = nil\n"
                                    "collectgarbage()\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_constructor_missing_required(void **state) {
  (void)state;

  lua_State *L = create_lua();
  register_instrument_target(L);

  assert_true(luaL_dostring(L, "local ok, err = pcall(function()\n"
                               "  instrument_target.new{}\n"
                               "end)\n"
                               "assert(ok == false)\n") == LUA_OK);

  lua_close(L);
}

static void test_lua_check_target_valid(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *original = instrument_target_create("i", "g", 1);

  push_target(L, original, 0);

  InstrumentTarget *extracted = lua_check_target(L, -1);

  assert_non_null(extracted);

  instrument_target_free(original);
  lua_close(L);
}

static void test_lua_check_target_non_userdata(void **state) {
  (void)state;

  lua_State *L = create_lua();

  lua_pushstring(L, "not a instrumenttarget");

  InstrumentTarget *extracted = lua_check_target(L, -1);

  assert_null(extracted);

  lua_close(L);
}

static void test_lua_check_target_wrong_metatable(void **state) {
  (void)state;

  lua_State *L = create_lua();

  // Create unrelated userdata
  void *ud = lua_newuserdata(L, sizeof(int));
  (void)ud;

  luaL_newmetatable(L, "NotInstrumentTarget");
  lua_setmetatable(L, -2);

  InstrumentTarget *extracted = lua_check_target(L, -1);

  assert_null(extracted);

  lua_close(L);
}

static void test_lua_check_target_null_internal(void **state) {
  (void)state;

  lua_State *L = create_lua();

  // Manually push userdata with NULL stack
  typedef struct {
    InstrumentTarget *stack;
    int owned;
  } lua_instrumenttarget;

  lua_instrumenttarget *cs =
      (lua_instrumenttarget *)lua_newuserdata(L, sizeof(lua_instrumenttarget));

  cs->stack = NULL;
  cs->owned = 0;

  luaL_newmetatable(L, "InstrumentTarget");
  lua_setmetatable(L, -2);

  InstrumentTarget *extracted = lua_check_target(L, -1);

  assert_null(extracted);

  lua_close(L);
}

static void test_lua_check_target_indexing(void **state) {
  (void)state;

  lua_State *L = create_lua();

  InstrumentTarget *cs = instrument_target_create("a", "", -1);

  push_target(L, cs, 0);

  lua_pushnumber(L, 42); // push extra value

  // InstrumentTarget is now at -2
  InstrumentTarget *extracted = lua_check_target(L, -2);

  assert_non_null(extracted);
  assert_string_equal(instrument_target_get_instrument_name(extracted), "a");

  instrument_target_free(cs);
  lua_close(L);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_lua_getters_basic),
      cmocka_unit_test(test_push_target_global),
      cmocka_unit_test(test_register_module),
      cmocka_unit_test(test_lua_owned_gc),
      cmocka_unit_test(test_lua_not_owned_gc),
      cmocka_unit_test(test_lua_null_safety),
      cmocka_unit_test(test_lua_to_string_method),
      cmocka_unit_test(test_lua_tostring_metamethod),
      cmocka_unit_test(test_lua_constructor_basic),
      cmocka_unit_test(test_lua_constructor_defaults),
      cmocka_unit_test(test_lua_clone_basic),
      cmocka_unit_test(test_lua_clone_independence),
      cmocka_unit_test(test_lua_clone_gc_owned),
      cmocka_unit_test(test_lua_constructor_missing_required),
      cmocka_unit_test(test_lua_check_target_valid),
      cmocka_unit_test(test_lua_check_target_non_userdata),
      cmocka_unit_test(test_lua_check_target_wrong_metatable),
      cmocka_unit_test(test_lua_check_target_null_internal),
      cmocka_unit_test(test_lua_check_target_indexing),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
