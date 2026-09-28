#include "instrument-target/instrument-target.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct InstrumentTarget {
  char instrument_name[INST_TARGET_MAX_STRING_LEN];
  char channel_group[INST_TARGET_MAX_STRING_LEN];
  int channel;
};

static void copy_string(char *dest, const char *src) {
  if (!dest)
    return;

  if (!src) {
    dest[0] = '\0';
    return;
  }

  strncpy(dest, src, INST_TARGET_MAX_STRING_LEN - 1);
  dest[INST_TARGET_MAX_STRING_LEN - 1] = '\0';
}

/* =========================
   Public API
   ========================= */

InstrumentTarget *instrument_target_create(const char *instrument_name,
                                           const char *channel_group,
                                           int channel) {
  InstrumentTarget *stack =
      (InstrumentTarget *)malloc(sizeof(InstrumentTarget));
  if (!stack) {
    return NULL;
  }

  copy_string(stack->instrument_name, instrument_name);
  copy_string(stack->channel_group, channel_group);
  stack->channel = channel;

  return stack;
}

void instrument_target_free(InstrumentTarget *stack) {
  if (stack) {
    free(stack);
  }
}

const char *
instrument_target_get_instrument_name(const InstrumentTarget *stack) {
  if (!stack)
    return NULL;
  return stack->instrument_name;
}

const char *instrument_target_get_channel_group(const InstrumentTarget *stack) {
  if (!stack)
    return NULL;
  return stack->channel_group;
}

const int instrument_target_get_channel(const InstrumentTarget *stack) {
  if (!stack)
    return -1;
  return stack->channel;
}

char *instrument_target_serialize(const InstrumentTarget *stack) {
  if (!stack)
    return NULL;

  // Worst-case size estimate:
  // 3 strings (64 each) + delimiters + int + null
  size_t max_size = 3 * INST_TARGET_MAX_STRING_LEN + 32; // safe margin

  char *buffer = (char *)malloc(max_size);
  if (!buffer)
    return NULL;

  snprintf(buffer, max_size, "%s|%s|%d", stack->instrument_name,
           stack->channel_group, stack->channel);

  return buffer;
}

InstrumentTarget *instrument_target_deserialize(const char *buffer) {
  if (!buffer)
    return NULL;

  InstrumentTarget *stack =
      (InstrumentTarget *)malloc(sizeof(InstrumentTarget));
  if (!stack)
    return NULL;

  memset(stack, 0, sizeof(InstrumentTarget));

  char instrument[INST_TARGET_MAX_STRING_LEN] = {0};
  char group[INST_TARGET_MAX_STRING_LEN] = {0};
  int channel = 0;

  int parsed =
      sscanf(buffer, "%63[^|]|%63[^|]|%d", instrument, group, &channel);

  if (parsed != 3) {
    free(stack);
    return NULL;
  }

  strncpy(stack->instrument_name, instrument, INST_TARGET_MAX_STRING_LEN - 1);
  strncpy(stack->channel_group, group, INST_TARGET_MAX_STRING_LEN - 1);

  stack->channel = channel;

  return stack;
}

InstrumentTarget *instrument_target_clone(const InstrumentTarget *stack) {
  if (!stack)
    return NULL;

  return instrument_target_create(instrument_target_get_instrument_name(stack),
                                  instrument_target_get_channel_group(stack),
                                  instrument_target_get_channel(stack));
}
