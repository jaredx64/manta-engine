#!/bin/sh

# Setup Directory
mkdir -p output

# Build Extension
vsce package -o output/vscmanta.vsix && code --install-extension output/vscmanta.vsix