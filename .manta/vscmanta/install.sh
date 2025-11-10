#!/bin/sh

mkdir extension
cp -R syntax       extension/syntax
cp -R extension.js extension
cp -R package.json extension

zip -rqm vscmanta.vsix extension
code --install-extension vscmanta.vsix