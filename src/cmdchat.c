#include "cmdchat.h"
#include <stdio.h>
#include <string.h>

typedef struct cmdchat_s
{
    char *buffer;
    uint32_t buffer_len;
    uint32_t username_start;
    uint32_t username_len;
    uint32_t msg_start;
    uint32_t msg_len;
} cmdchat_st;

#define CMDCHAT_PREFIX "[CHAT]"
#define CMDCHAT_MSG_PREFIX "#"
#define CMDCHAT_TOKEN_RESTART "restart"

const char *const cmdchat_prefix = CMDCHAT_PREFIX;
const uint16_t cmdchat_prefix_len = sizeof(CMDCHAT_PREFIX) - 1u;
const char *const cmdchat_msg_prefix = CMDCHAT_MSG_PREFIX;
const uint16_t cmdchat_msg_prefix_len = sizeof(CMDCHAT_MSG_PREFIX) - 1u;
const char *const cmdchat_token_restart = CMDCHAT_TOKEN_RESTART;
const uint16_t cmdchat_token_restart_len = sizeof(CMDCHAT_TOKEN_RESTART) - 1u;

static cmdchat_et cmdchat_parse_cmdchat(cmdchat_st cmdchat)
{
    bool username_found = false;
    bool msg_found = false;

    for (uint32_t cmd_idx = 0u; cmd_idx < cmdchat.buffer_len; ++cmd_idx)
    {
        if (cmdchat.buffer[cmd_idx] == ':')
        {
            cmdchat.username_start = 0u;
            cmdchat.username_len = cmd_idx;

            cmdchat.msg_start = cmdchat.username_start + cmdchat.username_len +
                                1 /* skip ':' */ + 1 /* skip ' ' */;
            if (cmdchat.msg_start < cmdchat.buffer_len)
            {
                /* Message is atleast 1 char long so length wont be negative (so
                 * won't wrap around). */
                cmdchat.msg_len = cmdchat.buffer_len - cmdchat.msg_start;
                if (cmdchat.msg_len >= cmdchat_msg_prefix_len + 1u &&
                    memcmp(&cmdchat.buffer[cmdchat.msg_start],
                           cmdchat_msg_prefix, cmdchat_msg_prefix_len) == 0)
                {
                    /* Message contains at least the command prefix and one
                     * char of command. */

                    // Don't include prefix in msg.
                    cmdchat.msg_start += cmdchat_msg_prefix_len;
                    cmdchat.msg_len -= cmdchat_msg_prefix_len;

                    msg_found = true;
                }
            }
            username_found = true;
            break;
        }
    }

    // Either message or username were not found.
    if (!username_found || !msg_found)
    {
        return CMDCHAT_UNKNOWN;
    }

    // TODO: Check username to see if it's allowed to run commands.
    printf("username: %.*s\n", cmdchat.username_len,
           &cmdchat.buffer[cmdchat.username_start]);
    printf("msg: %.*s\n", cmdchat.msg_len, &cmdchat.buffer[cmdchat.msg_start]);

    if (cmdchat.msg_len == cmdchat_token_restart_len &&
        memcmp(&cmdchat.buffer[cmdchat.msg_start], cmdchat_token_restart,
               cmdchat_token_restart_len) == 0)
    {
        printf("cmdchat: restart\n");
        return CMDCHAT_RESTART;
    }
    return CMDCHAT_UNKNOWN;
}

cmdchat_et cmdchat_parse_raw(char *buffer, const uint32_t buffer_len)
{
    bool cmdchat_found = false;
    uint32_t cmdchat_start_idx = 0u;

    for (uint32_t buf_idx = 0u; buf_idx < (buffer_len - cmdchat_prefix_len);
         ++buf_idx)
    {
        if (buffer[buf_idx] == '[')
        {
            if (memcmp(&buffer[buf_idx], cmdchat_prefix, cmdchat_prefix_len) ==
                0u)
            {
                cmdchat_found = true;
                cmdchat_start_idx = buf_idx;
                break;
            }
        }
    }

    if (!cmdchat_found)
    {
        return CMDCHAT_UNKNOWN;
    }

    uint32_t cmdchat_len = 0u;
    char cmdchat_buf[512u];
    for (uint32_t buf_idx = cmdchat_start_idx + cmdchat_prefix_len;
         buf_idx < buffer_len; ++buf_idx)
    {
        if (buffer[buf_idx] == '\n')
        {
            cmdchat_len = buf_idx - cmdchat_start_idx - cmdchat_prefix_len;
            break;
        }
    }

    if (cmdchat_len == 0u || cmdchat_len > 512u)
    {
        return CMDCHAT_UNKNOWN;
    }

    // This should contain a string like: "username: message"
    memcpy(cmdchat_buf, &buffer[cmdchat_start_idx + cmdchat_prefix_len],
           cmdchat_len);

    printf("raw: %.*s\n", cmdchat_len, cmdchat_buf);

    cmdchat_st cmdchat = {.buffer = cmdchat_buf, .buffer_len = cmdchat_len};
    return cmdchat_parse_cmdchat(cmdchat);
}
