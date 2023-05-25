# Binaural EBU ADM Renderer (BEAR)

The BEAR is a binaural renderer for ADM content, as specified in [EBU Tech 3396].
It is based on the [EAR], and is is integrated into the [EAR Production Suite].

This repository contains:

- visr_bear: a real-time C++ implementation of the BEAR, using [libear] and
  [VISR]. It has a C++ API that makes it reasonably straightforward to embed
  into other applications; see visr_bear/include/api.hpp

- bear: a python module containing a command-line tool `bear-render`, which
  uses visr_bear to render ADM WAV files to binaural.

This is currently a pre-release, awaiting code to generate the data files from publicly-available data and an EBU tech-doc describing the processing performed by the BEAR.

[EBU Tech 3396]: https://tech.ebu.ch/publications/tech3396
[EAR]: https://github.com/ebu/ebu_adm_renderer
[libear]: https://github.com/ebu/libear
[VISR]: https://github.com/s3a-spatialaudio/VISR
[EAR Production Suite]: https://ear-production-suite.ebu.io/

## Running with Docker

Building the BEAR can be challenging; as an alternative it's possible to run it
in docker if you just want to render files.

-   Install docker:

    -   On linux, install the docker package.

    -   On OSX or Windows, install docker desktop, or follow a guide
        [like this](https://medium.com/crowdbotics/a-complete-one-by-one-guide-to-install-docker-on-your-mac-os-using-homebrew-e818eb4cfc3)
        if the docker desktop license is not suitable.

-   Download the image, either:

    -   From a workflow artifact -- go to the
        [list of workflows](https://github.com/ebu/bear/actions/workflows/build.yml?query=branch%3Amain)
        , select one, and download `bear_docker.tar.gz` from the list of
        artifacts. It will come as a zip file which you'll have to extract (a
        github actions limitation).

    -   From [a release](https://github.com/ebu/bear/releases) -- future releases
        will include a `bear_docker.tar.gz` or similar file.

-   Load the image with:

        sudo docker load -i path/to/bear_docker.tar.gz

    On windows you probably don't need sudo.

To render a file in.wav to out.wav in the current directory:

    sudo docker run --rm -v $(pwd):/data bear bear-render data/in.wav data/out.wav

Or to see the available options, run:

    sudo docker run --rm bear bear-render --help

`-v` sets up a mapping between files inside the container and files outside (so
mapping the current directory (via `$(pwd)`) to `/data` means that `in.wav`
becomes `/data/in.wav` on the command-line); this is required because docker
runs processes in an isolated environment.

<details>
<summary>explanation of arguments</summary>

    sudo docker run

the command to run a container

    --rm

delete the created container (not the image) after is finishes

    -v $(pwd):/data

allow the container to access files in the current directory (see above)

    bear

the name of the image/container to run

    bear-render data/in.wav data/out.wav

The BEAR command-line to run.
</details>

## Native Install

To get this running, you'll need to install:

- This python module
- VISR
- visr_bear

Steps for each are outlined below.

To fetch the required submodules, first run:

    git submodule update --init --recursive

Installation in a virtualenv is generally preferable to doing a system-wide
install; follow the [Use a virtualenv](#use-a-virtualenv) steps below first to
do that.

### Python

This repository contains a standard python package.

Install it by changing into this directory and running:

    pip install -e .

For development you can install the test and dev dependencies with:

    pip install -e .[test,dev]

Tests are ran with:

    pytest

To reformat code to match the existing style, run `black` on any changed sources. `flake8` should ideally not report any warnings.

### VISR

Get VISR 0.13.0 or greater. Currently this is available here:

https://github.com/ebu/bear/releases/download/v0.0.1-pre/visr-0.13.0-pre-5e13f020.zip

Eventually this will be available from the upstream repository:

https://github.com/s3a-spatialaudio/VISR

To configure:

    cmake -B build . -DBUILD_PYTHON_BINDINGS=ON -DBUILD_DOCUMENTATION=OFF -DBUILD_AUDIOINTERFACES_PORTAUDIO=OFF -DBUILD_USE_SNDFILE_LIBRARY=OFF

Build:

    cmake --build build

Install:

    cmake --build build -t install

The cmake flags can be adjusted, but python bindings are currently necessary to
run `bear-render` and the tests.

### visr_bear

This is in the `visr_bear` subdirectory:

    cd visr_bear

The process is the same as for VISR; to configure:

    cmake -B build .

Build:

    cmake --build build

Install:

    cmake --build build -t install

### Adjustments

There are various ways to configure and build this:

#### Use a virtualenv

You should probably [use a virtualenv](https://packaging.python.org/guides/installing-using-pip-and-virtual-environments/#creating-a-virtual-environment)
to avoid messing up your system by installing things to your system directories
(which will require sudo). This is just a directory containing an installation
of python, and directories for installing libraries and programs.

Create it in `env` with:

    python -m venv env

To use the virtualenv you'll need to activate it; this just sets up some environment variables:

    source ./env/bin/activate

To get cmake to install into the virtualenv, add this to the cmake 'configuration' commands:

    -DCMAKE_INSTALL_PREFIX=$VIRTUAL_ENV

With this setup you'll probably need to add `env/lib` to your library search path; set this with:

    export LD_LIBRARY_PATH=$VIRTUAL_ENV/lib/

This could be added to the end of the `env/bin/activate` script if desired.

#### Release mode

For better performance, set either `-DCMAKE_BUILD_TYPE=Release` or
`-DCMAKE_BUILD_TYPE=RelWithDebInfo`. The latter is nice for development as it
retains debug symbols.

## Usage

### native ADM file renderer

    bear-render infile.wav outfile.wav

This supports various options similar to `ear-render`; see `bear-render --help`.

# License

Copyright 2020 BBC

BEAR is licensed under the terms of the Apache License, Version 2.0; see LICENSE for details.
