#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum cmdchat_e
{
    CMDCHAT_RESTART,
    CMDCHAT_BACKUP,
    CMDCHAT_UNKNOWN,
} cmdchat_et;

/**
 * @brief Parse raw buffer coming from DS into a chat command.
 * @param buffer
 * @param buffer_len
 * @return Chat command contained in the buffer.
 */
cmdchat_et cmdchat_parse_raw(char *buffer, const uint32_t buffer_len);
