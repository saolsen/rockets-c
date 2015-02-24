# Rockets
the c version, currently only works on osx

# Install Dependencies
* `brew install sdl2`
* `brew install premake` (A dependency of a dependency...)
* `git submodule init`
* `git submodule update --recursive`

* `cd nanovg`
* `premake4 gmake`
* `cd build`
* `make nanovg`
* `cd ../..`

# Make and run rockets
`make`

# Recompile any changes to game.c and reload while running.
`make`
