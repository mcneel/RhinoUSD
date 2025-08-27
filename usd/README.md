Precompiled source for https://github.com/PixarAnimationStudios/USD

Built from tag *v23.02*

# Building USD


## Getting source

Firstly checkout the correct source version

``` 
git checkout tags/v23.02
```

## Getting requirements

Install [UV](https://docs.astral.sh/uv/#projects)

You will already have CMake/C++ working on your OS if you can build Rhino.

``` bash
brew install tbb=2020.3
```

## Python Environment setup

``` bash
uv init
```

## Building Mac

Run this script, it will build to the v23.04 directory.

``` bash
uv run build_scripts/build_usd.py v23.04 --build-variant debug --build-monolithic --no-python --no-tests --no-examples --no-tutorials --no-tools
```

## Building Windows

In build_scripts directory, make win_debug directory and run the following python script for a Windows debug build

``` bash
python build_usd.py --generator "Visual Studio 16 2019" --build-variant debug --build-monolithic --no-python --no-tests --no-examples --no-tutorials --no-tools win_debug
```

## Failures

- If anything goes wrong with the python build script, the last line will give you a .txt file with errors (search for `error: `)
- If building is persistently wrong, changing versions when building will cause this, try building to a new folder
