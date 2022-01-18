# Space Impossible DS: Chat Commands
Add ability to execute additional commands from the chat of the game.

# Commands
|    CMD     |       Function       |
|:----------:|:--------------------:|
| `#restart` | Restarts the server. |
| `#backup`  | Backup the universe. |

# Usage
1. Run `make`.
2. Move `sids-cmdchat.elf` from the `build/` directory to the root of the server directory (this directory should contain the `SIDedicatedServer.x86_64` executable).
3. Copy `script/` directory to the root of the server directory.
4. Make sure the executable and all scripts in `script/` are executable (`chmod +x` all of them).
5. Run `sids-cmdchat.elf`.
