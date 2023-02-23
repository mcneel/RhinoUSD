Precompiled source for https://github.com/PixarAnimationStudios/USD

Built from tag *v23.02*

**Building USD**

In build_scripts directory, make win_debug directory and run the following python script for a Windows debug build

`python build_usd.py --generator "Visual Studio 16 2019" --build-variant debug --build-monolithic --no-python --no-tests --no-examples --no-tutorials --no-tools win_debug`