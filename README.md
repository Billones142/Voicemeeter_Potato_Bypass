# Voicemeeter Potato Bypass

thank you chat GPT. lol



# Compiler requirements
To compile this proyect is required having mingw 32bits and 64bits installed via MSYS2
For some reason g++ and gcc doesnt work if you're not compiling in the same directory, to fix this you will notice that in every script i move to the respective compiler and the move to the original directory (This is probably not a general problem with the compiler and its just my pc, but this workaround should still work for everyone)

# Bypass options

## VoicemeeterBypassInjector
### MinHook library


## VoicemeeterBypass.c (Currently not working on this)
The compiled code in the repo is compiled with all enabled.
Probably the voicemeeter devs will someday change this so idk how much time will it work, as the name says, its not a crack, its a bypass. If that ever happends you can try seaching the new addresses with cheat engine, or add an algorithm that searches automaticaly for the adresses, might be interesting doing it that way.
The program will wait 2 minutes to for voicemeeter to start.

This code have some definable preprocesor variables so you can customise what it does or what it doesnt. I thouhgt i might be cool doing it this way so that anyone can choose what it does, but maybe it's a little bit unnecesary.
It can (depending on what you enable):
  * change the time left variable to 0
  * change the function that changes the time left so that it always turns it to 0
  * log to console what is doing or what it did
  * log into a .log file
  * close the registration window
  * close the main window
  * close the drivers error window(in my case it appears, so i like this activated)