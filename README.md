# hanzi-ascii

This is just a little program I made for fun to display a 4 character chinese idiom (a chengyu) in the terminal. Right now it just prints to standard output but I'd like to refactor it to use ncurses.

It works by reading Unicode characters from a text file called 'hz', loading them in as freetype glyphs, rendering them as bitmaps at twice the print resolution, and then uses a cool sampling method to print them in monochrome ASCII using 16 different Unicode block elements. In this sense, it is perhaps misleading to call this "ASCII art", but I think it fits the spirit. 

My favorite part of this project is the way it samples 4 "pixels" at a time and uses bitwise operations in C to map each 2x2 grid to the corresponding block character without any branches or if statements of any kind. I felt quite proud after writing that :-)
