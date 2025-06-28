# sndhdump
a tool to dump the contents of the YM2149 registers and MFP registers of an SNDH.
# building
you should be able to run `make` and build a binary. it should also work on Windows with MinGW or WSL. 

i will figure out a way for users with MSVC later. if you have trouble compiling, open an issue and i will get to it (that is if i don't forget to check the repository). if you have a way to compile for another platform (MacOS, etc...), open a pull request and i'll look into it.

failing this, there are also binaries in the root of the repository which you can use.
# why?
because i can. this tool is inspired by [Lasse Oorni's siddump tool](https://github.com/cadaver/siddump) also i really needed a tool to look at the register contents of an SNDH while it is playing, but i don't quite like using a debugger to trace writes to the YM and MFP, and i tried to modify [SNDH-Archive-Player by Leonard/Oxygene](https://github.com/arnaud-carre/sndh-player) to do so, but i was not quite successful. luckily the AtariAudio part of the player can be used as an external library, so i modified it a little bit for easy YM and MFP register access.
the code is quite sloppy right now and i have not bothered to clean it up. regardless, if you want to contribute some fixes or changes, make a pull request! :)
# versions
- 0.1 : first version
# credits
- AtariAudio library written by Arnaud Carré aka Leonard/Oxygene
