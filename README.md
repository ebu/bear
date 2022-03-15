# Binaural EBU ADM Renderer (BEAR)

The BEAR is a binaural renderer for ADM content. It is based on the [EAR], and is is integrated into the [EAR Production Suite].

This repository contains:

- visr_bear: a real-time C++ implementation of the BEAR, using [libear] and
  [VISR]. It has a C++ API that makes it reasonably straightforward to embed
  into other applications; see visr_bear/include/api.hpp

- bear: a python module containing a command-line tool `bear-render`, which
  uses visr_bear to render ADM WAV files to binaural.

This is currently a pre-release, awaiting code to generate the data files from publicly-available data and an EBU tech-doc describing the processing performed by the BEAR.

[EAR]: https://github.com/ebu/ebu_adm_renderer
[libear]: https://github.com/ebu/libear
[VISR]: https://github.com/s3a-spatialaudio/VISR
[EAR Production Suite]: https://ear-production-suite.ebu.io/

## Install

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

Get VISR 0.13.0 or greater from:

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
