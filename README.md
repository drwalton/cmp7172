# cmp7172
Lab code for CMP7172 Real Time Graphics Programming

## Quick Start Guide

To get this code running, you should do the following:
1. Fork this repository on your own Github account
2. Clone the repository somewhere sensible (make sure you've got plenty of storage space)
3. Download all the necessary 3rdParty libraries (see `3rdParty/README.md`)
4. Run CMake on each individual lab session's code (remember to build binaries to a subdirectory called `build/`).
5. Open the generated solutions with Visual studio and build as usual.

To keep your repository up-to-date as new labs or bugfixes are released:
1. Open your forked repo on `github.com`.
2. Near the top of the page, under where you select the branch to view (e.g. `main`) you should see a commit report
3. This will say how many commits ahead of/behind this lab code repository you are.
4. If you're one or more commits ahead, that's fine and normal (that's due to the work you've done on the labs)
5. If you're one or more commits behind, that means there are new labs or new bugfixes for you to pull down!
6. To pull the changes, use the `Sync Fork` button to the right
7. This will pull the changes from this repo. Depending on what you've added you may need to do a git merge (ask me for help if you need it!).

## More Detailed Notes

When running cmake to generate your build files for each lab, do make sure to put them in a `build/` subdirectory.
As an example for the first lab session this would mean you'd build to `cmp7172/Week1/LinearAlgebra/build/` rather than just `cmp7172/Week1/LinearAlgebra/`.
This is standard practice when using CMake as it keeps your build & source files nicely separated.
This makes it way easier to e.g. exclude big build files from your git repo by adding `build/` to `.gitignore`.
Also if you don't structure your code in this way multiple aspects of the lab code may break, e.g. meshes and textures not found, DLLs not found and so on.

When building and debugging your lab exercises do make sure to select the project you're interested in (right click->`Set as startup project` in the solution explorer)

When debugging your applications in RenderDoc you'll have to set up the runtime directory and PATH environment variable correctly.
The runtime directory should be the `build/` directory for the particular lab you're working on.
To get the value you need to set `PATH` to you can check the project settings - right click on the project and check under `Properties->Debugging->Environment`. In renderdoc set `PATH` to the value after `PATH=` from Visual Studio.
