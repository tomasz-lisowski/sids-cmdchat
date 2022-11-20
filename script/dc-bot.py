import discord
import sys

class CMDChat(discord.Client):
    async def on_ready(self):
        print("CMDChat DC: ".format(self.user), file=sys.stdout)
        sys.stdout.flush()

    async def on_message(self, message):
        # Don't respond if it's our message
        if message.author == self.user:
            return

        if message.content[0] == '#' and len(message.content) <= 1024:
            print("{}".format(message.content), file=sys.stdout)
            sys.stdout.flush()
            await message.channel.send('CMDChat: Done')

client = CMDChat()
client.run('AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA')
