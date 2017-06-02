# emu8 - Chip8 VM Emulator for the terminal
You don't need no fancy OpenGL to intepret a VM from the 70's.

# Usage

You need ncurses for this.

  ~~~ sh
  $ git clone https://github.com/daedreth/emu8.git
  $ cd em8
  $ gcc emu8.c -o emu8 -lncurses
  $ ./emu8 <path to rom>
  ~~~

# Input
Similarly to the original, use the keys from 1 to 4, QWER for the second row,
ASDF for the third and YXCV for the last.

# What doesn't work
Occasional segfaults for some reason.

# Contributing
If you happen to find an issue, PR.
