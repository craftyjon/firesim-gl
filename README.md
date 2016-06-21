# firesim-gl
development on a stripped-down faster version of openlights/firesim

`firegl.py`: Python-based viewer, works except for some freezes
in fullscreen mode. Press `b` to switch blur mode on or off, `f`
to enter/exit fullscreen. Note that this is targeting Python 3.5
because that's what PyQt5 links against when you install it on
Homebrew. Changes to support Python 2 shouldn't be too bad (there's
some print_function and some usage of bytes() types in network
parsing.)

Other files: minimal start at making a C++ version of it.

