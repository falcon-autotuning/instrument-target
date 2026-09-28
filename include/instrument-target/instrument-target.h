#pragma once

/** Maximum length (including null terminator) for each string field */
#define INST_TARGET_MAX_STRING_LEN 64
#define INST_TARGET_SERIALIZED_MAX_SIZE (4 * INST_TARGET_MAX_STRING_LEN)

#ifdef __cplusplus
extern "C" {
#endif

#include "instrument-target/instrument-target-export.h"

/**
 * @brief Opaque handle representing an instrument target.
 *
 * @details The internal structure is hidden from users of the API.
 * Instances must be created and destroyed using the provided API functions.
 */
typedef struct InstrumentTarget InstrumentTarget;

/**
 * @brief Creates a new instrument target instance.
 *
 * @param instrument_name Name of the instrument (null-terminated string).
 * @param channel_group   Channel group identifier.
 * @param channel         Channel identifier.
 *
 * @return Pointer to a newly allocated InstrumentTarget instance, or NULL on
 * failure.
 *
 * @note All input strings are copied internally and truncated if they exceed
 *       INST_TARGET_MAX_STRING_LEN - 1 characters.
 */
INSTRUMENT_TARGET_EXPORT InstrumentTarget *
instrument_target_create(const char *instrument_name, const char *channel_group,
                         int channel);

/**
 * @brief Frees a InstrumentTarget instance.
 *
 * @param stack Pointer to the InstrumentTarget to free. Safe to pass NULL.
 */
INSTRUMENT_TARGET_EXPORT void instrument_target_free(InstrumentTarget *stack);

/**
 * @brief Retrieves the instrument name.
 *
 * @param stack InstrumentTarget instance.
 * @return Pointer to an internal null-terminated string.
 *
 * @warning The returned pointer is owned by the InstrumentTarget and must NOT
 * be modified or freed by the caller.
 */
INSTRUMENT_TARGET_EXPORT const char *
instrument_target_get_instrument_name(const InstrumentTarget *stack);

/**
 * @brief Retrieves the channel group.
 */
INSTRUMENT_TARGET_EXPORT const char *
instrument_target_get_channel_group(const InstrumentTarget *stack);

/**
 * @brief Retrieves the channel.
 */
INSTRUMENT_TARGET_EXPORT const int
instrument_target_get_channel(const InstrumentTarget *stack);

/**
 * @brief Serializes the call stack into a portable representation.
 *
 * @details The returned buffer contains a self-contained representation of the
 * InstrumentTarget that can be transmitted or stored and later reconstructed
 * using instrument_target_deserialize().
 *
 * @param stack InstrumentTarget instance.
 *
 * @return Pointer to a serialized buffer (null-terminated or binary-safe
 * depending on implementation).
 *
 * @note The returned memory is owned by the library and must not be modified.
 *       If dynamic allocation is used, a matching free API should also be
 * provided.
 */
INSTRUMENT_TARGET_EXPORT char *
instrument_target_serialize(const InstrumentTarget *stack);

/**
 * @brief Deserializes a InstrumentTarget from a serialized representation.
 *
 * @param buffer Serialized data.
 *
 * @return Pointer to a newly allocated InstrumentTarget instance, or NULL on
 * failure.
 */
INSTRUMENT_TARGET_EXPORT InstrumentTarget *
instrument_target_deserialize(const char *buffer);

/**
 * @brief Implements a deep copy of a InstrumentTarget instance.
 */
INSTRUMENT_TARGET_EXPORT InstrumentTarget *
instrument_target_clone(const InstrumentTarget *stack);

#ifdef __cplusplus
}
#endif
