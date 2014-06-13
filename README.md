hp4600
======

HP4600 scanner software for Linux

This software is dumb. I mean it doesn't really have any options and just replays the commands as a sort of blackbox magic incantation to produce a 600dpi color scan from the HP4600 scanner. No calibration, nothing else.

The software consists of two binaries, the USB replay and data capture and an image corrector.

The USB replay is, as I said, dumb.

The image corrector is required because the scanning elements on the scanner head have about a 7 pixel overlap and some offset. The image corrector fixes this. The colors will be slightly off in each bar which will be more or less noticable depending on the image scanned.

Good luck.

Dependencies:
Compiling the binaries requires libusb-devel.
The one step script uses Perl/Tk.

Compiling:
gcc hp4600-scanfullpage11.c -o hp4600scanfullfile -lusb
gcc -o fixhp4600output fixhp4600output.c
