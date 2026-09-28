#include <cmocka.h>
#include <stdarg.h>
#include <stddef.h>

#include <stdlib.h>
#include <string.h>

#include "instrument-target/instrument-target.h"

/* =========================
   Test Helpers
   ========================= */

static const char *VALID_NAME = "instr";
static const char *VALID_GROUP = "group";
static int VALID_CHANNEL = 2;

/* Generate long string (longer than max) */
static void fill_long_string(char *buf, size_t size) {
  for (size_t i = 0; i < size - 1; i++) {
    buf[i] = 'A' + (i % 26);
  }
  buf[size - 1] = '\0';
}

/* =========================
   CREATE + FREE TESTS
   ========================= */

static void test_create_valid(void **state) {
  (void)state;

  InstrumentTarget *cs =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  assert_non_null(cs);

  instrument_target_free(cs);
}

static void test_create_null_inputs(void **state) {
  (void)state;

  InstrumentTarget *cs = instrument_target_create(NULL, NULL, 0);
  assert_non_null(cs);

  assert_string_equal(instrument_target_get_instrument_name(cs), "");
  assert_string_equal(instrument_target_get_channel_group(cs), "");
  assert_int_equal(instrument_target_get_channel(cs), 0);

  instrument_target_free(cs);
}

static void test_free_null(void **state) {
  (void)state;

  /* Should not crash */
  instrument_target_free(NULL);
}

/* =========================
   GETTERS TESTS
   ========================= */

static void test_getters_valid(void **state) {
  (void)state;

  InstrumentTarget *cs =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  assert_string_equal(instrument_target_get_instrument_name(cs), VALID_NAME);
  assert_string_equal(instrument_target_get_channel_group(cs), VALID_GROUP);
  assert_int_equal(instrument_target_get_channel(cs), VALID_CHANNEL);

  instrument_target_free(cs);
}

static void test_getters_null_stack(void **state) {
  (void)state;

  assert_null(instrument_target_get_instrument_name(NULL));
  assert_null(instrument_target_get_channel_group(NULL));
  assert_true(instrument_target_get_channel(NULL) == -1);
}

/* =========================
   STRING TRUNCATION TEST
   ========================= */

static void test_truncation(void **state) {
  (void)state;

  char long_str[INST_TARGET_MAX_STRING_LEN * 2];
  fill_long_string(long_str, sizeof(long_str));

  InstrumentTarget *cs =
      instrument_target_create(long_str, long_str, VALID_CHANNEL);

  const char *name = instrument_target_get_instrument_name(cs);

  /* Ensure null-termination */
  assert_true(strlen(name) <= INST_TARGET_MAX_STRING_LEN - 1);

  /* Ensure last byte is null */
  assert_int_equal(name[INST_TARGET_MAX_STRING_LEN - 1], '\0');

  instrument_target_free(cs);
}

/* =========================
   SERIALIZATION TESTS
   ========================= */

static void test_serialize_valid(void **state) {
  (void)state;

  InstrumentTarget *cs =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  char *blob = instrument_target_serialize(cs);

  assert_non_null(blob);

  /* Ensure expected size behavior */
  /* We cannot directly check size, but we know it's fixed */
  free(blob);
  instrument_target_free(cs);
}

static void test_serialize_null(void **state) {
  (void)state;

  const char *blob = instrument_target_serialize(NULL);
  assert_null(blob);
}

/* =========================
   DESERIALIZATION TESTS
   ========================= */

static void test_deserialize_valid(void **state) {
  (void)state;

  InstrumentTarget *original =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  char *blob = instrument_target_serialize(original);
  assert_non_null(blob);

  InstrumentTarget *copy = instrument_target_deserialize(blob);
  assert_non_null(copy);

  assert_string_equal(instrument_target_get_instrument_name(copy), VALID_NAME);
  assert_string_equal(instrument_target_get_channel_group(copy), VALID_GROUP);
  assert_int_equal(instrument_target_get_channel(copy), VALID_CHANNEL);

  free(blob);
  instrument_target_free(original);
  instrument_target_free(copy);
}

static void test_deserialize_null(void **state) {
  (void)state;

  InstrumentTarget *cs = instrument_target_deserialize(NULL);
  assert_null(cs);
}

/* =========================
   ROUNDTRIP TEST
   ========================= */

static void test_roundtrip(void **state) {
  (void)state;

  InstrumentTarget *original =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  char *blob = instrument_target_serialize(original);
  InstrumentTarget *copy = instrument_target_deserialize(blob);

  assert_string_equal(instrument_target_get_instrument_name(copy),
                      instrument_target_get_instrument_name(original));

  assert_string_equal(instrument_target_get_channel_group(copy),
                      instrument_target_get_channel_group(original));

  assert_int_equal(instrument_target_get_channel(copy),
                   instrument_target_get_channel(original));

  free(blob);
  instrument_target_free(original);
  instrument_target_free(copy);
}

/* =========================
   CORRUPTED BUFFER TEST
   ========================= */

static void test_deserialize_corrupted(void **state) {
  (void)state;

  char *blob = malloc(INST_TARGET_SERIALIZED_MAX_SIZE);
  assert_non_null(blob);

  /* Fill with garbage */
  memset(blob, 0xFF, INST_TARGET_SERIALIZED_MAX_SIZE);

  InstrumentTarget *cs = instrument_target_deserialize(blob);
  assert_null(cs);

  free(blob);
  instrument_target_free(cs);
}

static void test_serialization_is_string_safe(void **state) {
  (void)state;

  InstrumentTarget *cs =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  char *blob = instrument_target_serialize(cs);
  assert_non_null(blob);

  size_t len = strlen(blob);

  assert_true(len > 0);

  for (size_t i = 0; i < len; i++) {
    assert_true(blob[i] != '\0');
  }

  assert_int_equal(blob[len], '\0');

  assert_non_null(strchr(blob, '|'));

  assert_non_null(strstr(blob, VALID_NAME));
  assert_non_null(strstr(blob, VALID_GROUP));

  free(blob);
  instrument_target_free(cs);
}

static void test_string_safe_roundtrip(void **state) {
  (void)state;

  InstrumentTarget *original =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  char *blob = instrument_target_serialize(original);
  InstrumentTarget *copy = instrument_target_deserialize(blob);

  assert_non_null(copy);

  assert_string_equal(instrument_target_get_instrument_name(copy), VALID_NAME);

  assert_string_equal(instrument_target_get_channel_group(copy), VALID_GROUP);

  assert_int_equal(instrument_target_get_channel(copy), VALID_CHANNEL);

  free(blob);
  instrument_target_free(original);
  instrument_target_free(copy);
}

static void test_string_transport_safe(void **state) {
  (void)state;

  InstrumentTarget *original =
      instrument_target_create(VALID_NAME, VALID_GROUP, VALID_CHANNEL);

  char *blob = instrument_target_serialize(original);
  assert_non_null(blob);

  size_t len = strlen(blob);

  InstrumentTarget *copy = instrument_target_deserialize(blob);

  assert_non_null(copy);

  assert_string_equal(instrument_target_get_instrument_name(copy), VALID_NAME);

  assert_string_equal(instrument_target_get_channel_group(copy), VALID_GROUP);

  assert_int_equal(instrument_target_get_channel(copy), VALID_CHANNEL);

  free(blob);
  instrument_target_free(original);
  instrument_target_free(copy);
}
static void test_deserialize_invalid_format(void **state) {
  (void)state;

  const char *bad = "not|enough";

  InstrumentTarget *cs = instrument_target_deserialize(bad);

  assert_null(cs);
}
static void test_deserialize_empty_string(void **state) {
  (void)state;

  InstrumentTarget *cs = instrument_target_deserialize("");

  assert_null(cs);
}

/* =========================
   MAIN
   ========================= */

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_create_valid),
      cmocka_unit_test(test_create_null_inputs),
      cmocka_unit_test(test_free_null),

      cmocka_unit_test(test_getters_valid),
      cmocka_unit_test(test_getters_null_stack),

      cmocka_unit_test(test_truncation),

      cmocka_unit_test(test_serialize_valid),
      cmocka_unit_test(test_serialize_null),

      cmocka_unit_test(test_deserialize_valid),
      cmocka_unit_test(test_deserialize_null),

      cmocka_unit_test(test_roundtrip),
      cmocka_unit_test(test_deserialize_corrupted),
      cmocka_unit_test(test_deserialize_corrupted),
      cmocka_unit_test(test_deserialize_invalid_format),
      cmocka_unit_test(test_serialization_is_string_safe),
      cmocka_unit_test(test_string_safe_roundtrip),
      cmocka_unit_test(test_string_transport_safe),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
