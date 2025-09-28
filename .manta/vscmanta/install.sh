#!/bin/sh

mkdir extension
cp -R syntax       extension/syntax
cp -R extension.js extension
cp -R package.json extension

zip -rqm custom.vsix extension
code --install-extension custom.vsix && rm custom.vsix