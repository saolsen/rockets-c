# Rockets
the c version, currently only works on osx

## Install Dependencies
* `brew install sdl2`
* `brew install premake` (A dependency of a dependency...)
* `git submodule init`
* `git submodule update --recursive`

* `cd nanovg`
* `premake4 gmake`
* `cd build`
* `make nanovg`
* `cd ../..`

## Make and run rockets
`make`

## Run the game
`./rockets`

## Recompile any changes to game.c and reload while running.
`make`

# changelog
* [before now]    Did a ton of work. Starting this changelog pretty deep into the process of developing this. I've built out a prototype of just about everything.

* [8/21/15]    Getting back into development here. I learned a lot of c since I last worked on this so the first thing I'm doing is dropping the stb stretchy buffer stuff. I decided I don't need it and I'm going to get rid of dynamic memory allocation for this. I want to redo the node stuff to also use up front static allocation and a free list. Gonna do that next (maybe today). The biggest things I need to tackle with this is how the UI works. It was all written by me very adhocily and it's not a very good user experience. I had nate try it out a few weeks ago and he had no idea what any of the buttons did (which makes sense because they are all just blue). Dragging connections between nodes is very non obvious so that's something I need to make nicer. I think I just need more hints on things when you scroll over them, etc. Gonna maybe just do a new version from scratch once I modify the node api anyway. I need there to be math in the nodes I think so you can calculate things. It's hard to say really what all should be available in the nodes. I wonder if there's some logic system that embodies all the actions one would want to take that I can map easily to a set of node rules.

* [8/21/15]    Did a pretty big rewrite of the nodestore. It now uses a fixed amount of memory and a freelist to manage instead of using malloc and free. That along with removing stb stretchy buffers means this is completely manual memory management again, only one allocation at startup, casey style. I like this a lot now that I know how to use it. I also added an actual hash map for indexing by id. I also got how to implement this from watching casey. I will probably also add a topological sort of the nodes at some point so that evaluation can be faster. Next I think I should spend some time on level designing and collision/entity stuff so that I can play with some more complicated levels and decide how I want the node stuff to actually work.