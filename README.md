# Semantic Text Clock

This code is used in a project of mine where I build a semantic text clock that displays the time in swiss german. The corresponding 3D files for the hardware, and further information about electrical components and wiring can be found on my thingiverse page:

https://www.thingiverse.com/posa1212/designs

![alt text](https://github.com/smaassen/SemanticTextClock/blob/master/doc/images/Textclock_2.png "Rendering")

The code provided works for an led chain of 42 (roughly 1 LED for 2 letters that need to be illuminated), but it can be adapted by changing the follwing things:

1. Change `NUMPIXELS` definition in line 8
2. Change the "time to LED" mapping defined in function `setLEDStates(int states[NUMPIXELS], int hour,int minutes)`

The clock is able to display all type of rgb color. Colors, not included in the stock colorscheme can be appended to the `colorScheme[N_colors][3]` array (line 56)


The code was tested on an Arduino NANO.
