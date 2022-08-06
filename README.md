# Lith-Game

A game written from scratch in c style c++, it is windows only for now but I have plans on proting it
to other platforms if I find time for it

This was my first time implementing a networked game and it was a big learning experience, this
project had a lot of experimentations and trial and error and certainly lot of bugs and stupid
mistakes, but it was fun

# Game Rules:

Lith is a card game, where a deck of 52 cards is split between 4 players, a player declare a card
number and puts it to the table face down, the next player can either play a card of supposedly the
same number, or call the previous player a liar, if the previous player was lying about their card
number he takes all the cards in the table, if he was telling truth the player who called him liar
takes all the cards on the table.

the first player who empty their hands wins.

# Build
to build on windows make sure you have cl.exe accessible from your terminal, then run build.bat
this will generate a build folder, inside it you find the game executable `Lith.exe`


# TODOs:
- [x] Add config file to load lobby ip address and such
- [ ] Add Audio
- [ ] Port to other platforms
- [ ] Host the lobby somewhere
