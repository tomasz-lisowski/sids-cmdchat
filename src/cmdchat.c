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
#define CMDCHAT_PREFIX_LEN ((uint32_t)sizeof(CMDCHAT_PREFIX) - 1u)

#define CMDCHAT_MSG_PREFIX "#"
#define CMDCHAT_MSG_PREFIX_LEN ((uint32_t)sizeof(CMDCHAT_MSG_PREFIX) - 1u)

#define CMDCHAT_TOKEN_RESTART "restart"
#define CMDCHAT_TOKEN_RESTART_LEN ((uint32_t)sizeof(CMDCHAT_TOKEN_RESTART) - 1u)

#define CMDCHAT_TOKEN_BACKUP "backup"
#define CMDCHAT_TOKEN_BACKUP_LEN ((uint32_t)sizeof(CMDCHAT_TOKEN_BACKUP) - 1u)

static bool cmdchat_iscmd(const cmdchat_st cmdchat, const char *const token,
                          const uint32_t token_len)
{
    return cmdchat.msg_len == token_len &&
           memcmp(&cmdchat.buffer[cmdchat.msg_start], token, token_len) == 0;
}

static cmdchat_et cmdchat_parse_chat(cmdchat_st cmdchat)
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
                                2 /* skip ': ' */;
            if (cmdchat.msg_start < cmdchat.buffer_len)
            {
                /* Message is atleast 1 char long so length wont be negative (so
                 * won't wrap around). */
                cmdchat.msg_len = cmdchat.buffer_len - cmdchat.msg_start;

                if (cmdchat.msg_len >= CMDCHAT_MSG_PREFIX_LEN + 1u &&
                    memcmp(&cmdchat.buffer[cmdchat.msg_start],
                           CMDCHAT_MSG_PREFIX, CMDCHAT_MSG_PREFIX_LEN) == 0)
                {
                    /* Message contains at least the command prefix and one
                     * char of command. */

                    // Don't include prefix in msg.
                    cmdchat.msg_start += CMDCHAT_MSG_PREFIX_LEN;
                    cmdchat.msg_len -= CMDCHAT_MSG_PREFIX_LEN;

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
        printf("!username_found || !msg_found (%u %u)\n", username_found,
               msg_found);
        return CMDCHAT_UNKNOWN;
    }

    // TODO: Check username to see if it's allowed to run commands.
    printf("Username: %.*s\n", cmdchat.username_len,
           &cmdchat.buffer[cmdchat.username_start]);
    printf("Msg: %.*s\n", cmdchat.msg_len, &cmdchat.buffer[cmdchat.msg_start]);

    if (cmdchat_iscmd(cmdchat, CMDCHAT_TOKEN_RESTART,
                      CMDCHAT_TOKEN_RESTART_LEN))
    {
        printf("CMDChat: " CMDCHAT_TOKEN_RESTART "\n");
        return CMDCHAT_RESTART;
    }
    if (cmdchat_iscmd(cmdchat, CMDCHAT_TOKEN_BACKUP, CMDCHAT_TOKEN_BACKUP_LEN))
    {
        printf("CMDChat: " CMDCHAT_TOKEN_BACKUP "\n");
        return CMDCHAT_BACKUP;
    }
    return CMDCHAT_UNKNOWN;
}

cmdchat_et cmdchat_parse_raw(char *buffer, const uint32_t buffer_len)
{
    bool cmdchat_found = false;
    uint32_t cmdchat_start_idx = 0u;

    if (buffer_len >= CMDCHAT_PREFIX_LEN)
    {
        for (uint32_t buf_idx = 0u; buf_idx < (buffer_len - CMDCHAT_PREFIX_LEN);
             ++buf_idx)
        {
            if (buffer[buf_idx] == '[')
            {
                if (memcmp(&buffer[buf_idx], CMDCHAT_PREFIX,
                           CMDCHAT_PREFIX_LEN) == 0u)
                {
                    cmdchat_found = true;
                    cmdchat_start_idx = buf_idx;
                    break;
                }
            }
        }
    }

    if (!cmdchat_found)
    {
        return CMDCHAT_UNKNOWN;
    }

    uint32_t cmdchat_len = 0u;
    char cmdchat_buf[512u];
    for (uint32_t buf_idx = cmdchat_start_idx + CMDCHAT_PREFIX_LEN;
         buf_idx < buffer_len; ++buf_idx)
    {
        if (buffer[buf_idx] == '\n')
        {
            cmdchat_len = buf_idx - cmdchat_start_idx - CMDCHAT_PREFIX_LEN;
            break;
        }
    }

    if (cmdchat_len == 0u || cmdchat_len > 512u)
    {
        return CMDCHAT_UNKNOWN;
    }

    // This should contain a string like: "username: message"
    memcpy(cmdchat_buf, &buffer[cmdchat_start_idx + CMDCHAT_PREFIX_LEN],
           cmdchat_len);

    printf("RawChat: %.*s\n", cmdchat_len, cmdchat_buf);

    cmdchat_st cmdchat = {.buffer = cmdchat_buf, .buffer_len = cmdchat_len};
    return cmdchat_parse_chat(cmdchat);
}
