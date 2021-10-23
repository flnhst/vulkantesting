#!/usr/bin/env bash

GLSLANG_VALIDATOR="/home/florian/varfast/build/glslang/commit_e56beaee736863ce48455955158f1839e6e4c1a1/install_release/bin/glslangValidator"

$GLSLANG_VALIDATOR shaders/shader.vert -V -o spv/vert.spv
$GLSLANG_VALIDATOR shaders/shader.frag -V -o spv/frag.spv
