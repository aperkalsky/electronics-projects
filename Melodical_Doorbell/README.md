A melodical doorbell developed back in 1997-98.

Upon pressing a button it randomly picks a melody (from the predefined set) adn plays it.

A software pseudo-random generator is used to pick a melody.

The hardware is based on 8032 microcontroller, the code written in assembly language.
Built on PC using asm51 toolchain.

The melody snippets were obtained by converting MIDI files to TAB format (table of frequency and duration)
Then the TAB data was converted to the lookup tables usable by assembly code. The conversion tools were
developed too.

