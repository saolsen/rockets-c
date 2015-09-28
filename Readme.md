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

# The life and times of steveo as depicted in the course of development of a new and most novel game
* [before now]    Did a ton of work. Starting this changelog pretty deep into the process of developing this. I've built out a prototype of just about everything.

* [8/21/15]    Getting back into development here. I learned a lot of c since I last worked on this so the first thing I'm doing is dropping the stb stretchy buffer stuff. I decided I don't need it and I'm going to get rid of dynamic memory allocation for this. I want to redo the node stuff to also use up front static allocation and a free list. Gonna do that next (maybe today). The biggest things I need to tackle with this is how the UI works. It was all written by me very adhocily and it's not a very good user experience. I had nate try it out a few weeks ago and he had no idea what any of the buttons did (which makes sense because they are all just blue). Dragging connections between nodes is very non obvious so that's something I need to make nicer. I think I just need more hints on things when you scroll over them, etc. Gonna maybe just do a new version from scratch once I modify the node api anyway. I need there to be math in the nodes I think so you can calculate things. It's hard to say really what all should be available in the nodes. I wonder if there's some logic system that embodies all the actions one would want to take that I can map easily to a set of node rules.

* [8/21/15]    Did a pretty big rewrite of the nodestore. It now uses a fixed amount of memory and a freelist to manage instead of using malloc and free. That along with removing stb stretchy buffers means this is completely manual memory management again, only one allocation at startup, casey style. I like this a lot now that I know how to use it. I also added an actual hash map for indexing by id. I also got how to implement this from watching casey. I will probably also add a topological sort of the nodes at some point so that evaluation can be faster. Next I think I should spend some time on level designing and collision/entity stuff so that I can play with some more complicated levels and decide how I want the node stuff to actually work.

* [8/24/15]    I broke out the predicate nodes into actually having the signals and constants to be their own nodes and updated the GUI code to handle it. This is mostly to enable some future math nodes. I still have a lot of work to do on the GUI to make it nicer to use. I'm going to add some sliders and things for modifying values and show visual indicators when dragging inputs and outputs to show where you can drag them to.

* [8/26/15]    I did a first pass at a slider for the constant nodes. It seems to work but it has a lot of problems. 1 The code is pretty fragile. I'm having a lot of trouble seeing how I can actually take this UI code and turn it into something people are actually going to want to use. More and more I think I should use a real GUI toolkit like the browser to do all of this. The other big problem with this slider though is I can't hit every number on the spectrom. Unless the slider were 700 pixels long I can't actually toggle it to every number between 1 and 700. That's a problem because you probably have a specific value in mind you are trying to hit. I also don't want to just have the slider go 0 - 700 because for some things, like rotations that doesn't even make sense. The value you are toggeling should really depend on the nodes that depend on this constant node. UI stuff is very frustrating, this thing would be so easy in mathematica's UI toolkit, really wishing that was a library for c.

* [8/30/15]    Slider not being wide enough is sort of a deal breaker so I'm moving to buttons. I'm going to have an up and down button, and you can click them to change the value by 1 or hold them to change the value faster. The longer you hold it the faster the value changes so you can get around quickly then click it to hone in on what you want. This is a pretty common UI thing, roughly based on how this works. https://jqueryui.com/spinner/ I've also spent a lot of time thinking about how INGUI style ui could be done in larger apps. Most imgui libraries are just for debug or developer tools, not for player facing UI but I don't want to move to a "REAL" ui library so I have some ideas for ways to do widgets. I want to not have callbacks so I will have a little queue of events the player can loop through for stuff that happened that frame, I also want to have more sytling power, maybe build a flexbox layout model too. (I think that is what facebook is doing for react native and nikki was showing me it and it looked to be simple enough to do from scratch) This is all getting ahead of myself a bit, need to finish rockets and then will probably do a lot of this AI work on my AI game playing tools if I ever get to those.

* [9/01/15]      Today was mostly procrastination. I got jelous of how good sublime is for golang and wanted to use it for c. I spent a long time figuring out how to do that and finding the sublime-clang plugin that actually makes this a pretty great IDE. One problem though was that it only seems to support c++, not c and I already kind of wanted operator and method overloading so I just switched the project over to c++. It works the same as it did before. I threw in some changes to my vector code that are enabled by the switch to c++ but it isn't really that much of a change.

* [9/01/15]      That was a bad move. I broke my debugger and sublime isn't even that good and emacs is fine and I just reverted everything and am back in C.

* [9/01/15]      Wrote an entity system and ported all the adhoc goal and ship handling code to use it. This is going to let me treat all the things in the scene in common ways and enable me to write some good collision detection code. This is similar to the way casey does stuff except a lot simpler because I don't have a high and low frequency update set or sim regions or any of that stuff. Everything in the scene is an entity. I'm going to now set velocity on the entities and write a general collision detection routine to handle moving them around. This way I can have planets move in the scene too. I'm going to need to do some work on fixing my timestep because I want this to all be deterministic, hitting replay with the same nodes should act the same way regardless of the framerate. Feeling really good about this!

* [9/03/15]      Today I finially got around to writing collision detection. This was pretty fun. I used minkownski sums. I did a pretty simple version only dealing with AABB boxes. I think this is a good start. Next I need to allow for rotations and then also add in other possible shapes. Debugging this was pretty hard because I can't draw to the space scene from update code because the transform is wrong. I should either do all my space scene updates from within the right nvg transform or write some helper functions for this. I ended up fighing a lot of suble bugs in my 4 deep nested for loop. Things that even just having a foreach would have avoided. C is sort of tedious in that regard. All in all it went really well though and now I can design some levels. I want to have finding the goal, not hitting walls and running into objects in the scene all use the collision system. I didn't have to do much math about collision handling because if you hit something you either win or you die, I only had to write collision detection code. I will do a bit of collision handling but only to react to things like winning or losing, I wont be doing any "find where the player should be placed" math for this, I'll save that for the next game.

* [9/04/15]      Been watching handmade hero's debug stuff and I'm really excited about building some of that for rockets. A lot of the power he gets from that comes from the way his project is organized so I did some refactoring to do a unity build in the same style. I have my rockets.h file include all the other .h files and my rockets.cpp file include all the other cpp files. Now that it's one compilation unit I'll get some benfits when implementing casey's performance stuff. One side effect of this though was that .cpp files don't include the .h files they depend on which isn't a problem for the build but is a problem for running flymake on individual files. To fix this I just #include rockets.h in all cpp files. Theres an include guard so this doesn't do anything for the real build but lets flymake still work on individual files. I pulled out a couple "namespaces" from the main file too and am working on seperating things. The next big step is to factor out the renderer into something that doesn't immediately draw to the screen and build some debug services. This way I can drop nanovg for the real renderer at some point and I can make sure debug stuff always draws on top of the scene without having to do all debug stuff at the bottom. I will build up logs of things and some gui to explore them. Should be really helpful. I found another error in my collision detection but I want to get some debug stuff set up before I tackle it so it's a lot easier to figure out.

* [9/04/15]      I did a ton of work to set up the debug stuff for rockets. I have a way to collect performance counters and a way to draw debug vectors and boxes on the scene. I really like the way the performance counters are collected. I still have to figure out some of the math to make that data useful and come up with a good way to present it. There are a lot of places I'd like to go with this stuff like replaying the game from a checkpoint with the recorded input and stepping through what happens, detecting slow frames. There's a lot I'd like to do with the debug drawing too like having a UI where I can turn on and off debug drawing for certain things on certain entities, stuff like that. All of that is a lot of work and it's hard to say if it's worth doing, I don't really know what sorts of things I'm really going to end up needing to debug. I have a good start though and will be adding stuff as I need it.

* [9/08/15]      Slowly getting back into this after a long weekend. I think I'm in the middle of debugging the collision detection but I just took a sec to print out better statistics for the performance counter stuff. This is going to be helpful later but I will probably display it as a searchable table in game rather than interact with it as a console log. That way I can corrilate it to other debug info and replay slow frames, etc.

* [9/09/15]      I was thinking about the game and all the things I want to do for it but what's really important is that it's fun and the puzzles are good. If the puzzles aren't good and the game isn't fun then any other platform / debugger / story / graphics / ui doesn't even matter so I need to refocus on making good levels. I started that by adding an easier way to programattically build levels. I introduced a new data structure to represent a level specification. There are going to be a rich set of functions to build these up. Then when the user loads a level these specs are translated into actuial entities. In that way we make little prefabs (using a unity term) that let me specify a ship is at this location, but when the level is loaded the actual collision geometry and stuff is set up correctly. These are live editable the same way my other code is so I should be able to iterate on level design this way. This will let me really get to work on the core of the game which has to be solid or everything else is a waste of time.

* [9/09/15]      I'm starting to design puzzles now. I really don't like the design workflow I have. It's very hard to use and to visualize what I want the player to be able to do. It might work for awhile but I think I need to design a level editor. I've been making some progressively challenging levels that try to make the player learn a new way of flying the ship for each level. I don't really know how far something like this can be taken but that's really the mvp I need to create to see if this is even a viable game. I ran into some problems on level 5 though and that collision detection bug is stopping that level from working so I need to take a bit of time and fix that before I can progress.

* [9/11/15]     I guess here's a good place to commit some of the "New Rockets" I'm changing the way the game works to operate on a grid instead of in free space. I think that will simplify some of the reasoning the player has to do to figure out how far the ship will go when they set thrusters. I've started with some grid drawing code and started working out the rocket motion. I need to do some work to switch to cube coordinates so the math is easier and then I have to figure out what sort of impact this has on the nodes. I havn't completely decided this is going to work yet but I'm going down this path for mow. I didn't like the absolute positioning math you had to do in the old puzzles and hope this leads to something more relative to the ship instead of to it's place in space. Also because the simulation is now going to be discrete and deterministic stuff like multiplayer battles is going to be possible and that's very exciting. Just need to get it to be fun first though.

* [9/12/15]     I'm in the middle of switching to cube coordinates. It's a much nicer way to do do things. I have a few bugs still but I think I'll be good soon. One thing I did was I have a hardcoded lookup table for how the rocket will move, and with the new cube coordinates stuff like moving up 2 spaces is just adding the UP vector twice, but because this table is built at compile time I can't do that math directly and have just hardcoded a bunch more vectors. That's a bummer and would be easily solved if C actually had macros instead of just a bad pre processor.

* [9/13/15]     Fixed the bugs in my math. I had one very strange bug where things would show up sometimes and not other times and that sometimes math stuff would return nan. Now I realise that's a pretty sure case of uninitialized data (and it was). Hopefully I'll recognize that sooner next time. I have the grid movement code all using cubic coordinates now. I can start working on the nodes and on maybe designing a simpler to use ship. Feeling good about this.

* [9/13/15]     Did some organization work. Really trying to keep the rendering and gui stuff sane this time and isolated to their specific files. I think I hae a pretty good base now and I can start prototyping some new node stuff.

* [9/15/15]     I rewrote the grid rendering code this morning. I don't really think it's any better now, I was really just trying to do something small and simple this morning. The new version I tried to get it to not duplicate writing any lines but I new have lots of conditional stuff in inner loops and I don't think it's good. Seems like the sort of thing mike actin is super against. Need to revisit it at some point and see if I can write it better. Would be interesting to see what asm is spit out and use that as a guide to refactor (that and real profiling once I get the debug stuff set up again)

* [9/15/15]     I did a ton of work on the gui system today. I am modeling it after the couple IMGUI projects I know about. I save all the drawing information in a command buffer to render at the end of the frame but I check for click and drag events, etc as they are asked for. I really like this so far, it's going well. Still needs some work though.

* [9/16/15]     I've been real confused and conflicted about how to deal with memory. I'm trying to understand casey's use of memory areas (which are just stacks) Casey has a data structure for an area and you can create sub areas. This is just a stack and you can pop the whole sub stacks off at a time. It work really well for the type of memory you need to calculate something. This is the kind of thing I'll use for my buffers for rendering and gui. I wrote the logic in the gui file but will probably pull it out to be used by other things too. I need to find out how the area stuff relates to object pools (do they use areas or only fixed size arrays) and other general purpose allocation stuff. Also need to figure out how to have a memory area that can grow and shrink and interact with the system allocator. In the talk from last night about slab allocators he made the case that you should never do your own allocation stuff and just use the system allocator because the slab allocator is really good and will pack similar sized objects together anyway. That sounds like a good thing for server or os software but I think in a game I should really learn the game way of doing things which is doing it yourself.

* [9/17/15]     Really need to get going on the nodes and the rules and the gameplay again. Gotta get back to a playable demo so I can see if it's fun. I started by doing some work to get the nodestore back online. I think the structure of the nodes is going to be slightly different this time. Need to figure out this radar/sensor idea. I started working out the gui stuff for it but ran into a bug with my gui pushbuffer that I think is because I'm not handling alignment when pushing structs so I need to figure that out. This is my first real memory error bug.

* [9/17/15]     Oh man! That bug was hard so I went back and rewatched all the stuff on how casey initializes and uses the memory for his game. I'm now doing something in a similar way where I have a large arena up front. I then use that arena to initialize the command buffer for the gui system which then manages it's block itself. This is pretty nice now. I got rid of the global gui_state pointer too and am going to get rid of the one in the renderer. I should just be explicit.

* [9/18/15]     Started today by modifying the nodestore to use the arena too. I need to figure out how I'm going to actually determine how much memory these things get in the real game but for the prototyping it doesn't really matter.

* [9/18/15]     I began working on the gui code for the nodes. I think this system is a lot cleaner. I'm still hacking out how it's going to work but it seems like the IMGUI approach is going to be a lot nicer than what I did in the first prototype.

* [9/21/15]     Did a ton of work on the node ui today. Got the first two types mostly set up and figured out some drag and drop stuff. This UI is tricky to figure out but seems a lot nicer to work with than prototype 1. I think it's pretty clean. The hardest part is figuring out the api and when to check things. I have a few things that will take a frame of lag because they take a whole frame of computation to figure out, I think that's just fine though. Next I have to keep working out the data on the node structs and how it works into the gui, these things are good to develop in parallel. Next step is some arow drawing I think for drag states and for node connections. Will try to use a beizer curve this time instead of just a line.

* [9/23/15]     Did a bunch of work on the scene now. I have the tick mechanism set up and all the info I need to do some interpolated animation later. I need to make some real lose/win conditions and design a few levels. The next real big thing to do is the node evaluation and using that to drive the rocket. Then it's just a lot of polish and some playable demo levels and I can start showing it to people.

* [9/28/15]     Fixed an iteration bug in my node evaluation queue. I have a system that lets me evaluate the nodes in topological order but there was some iteration through my queue bug that was being very tricky. I currently can't get gdb to work with my project which is really anoying and makes any debugging rather difficult. I need to figure out how to debug this project properly.