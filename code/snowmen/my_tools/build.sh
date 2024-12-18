#!/usr/bin/env bash

set -e

# Tools
echo "Building tools..."
make -C gltf_collision_importer -j4
make -C gltf_collision_importer install || sudo -E make -C gltf_collision_importer install


echo "Build done!"