@echo off

:: Setup Directory
if not exist output mkdir output

:: Build Extension
vsce package -o output\vscmanta.vsix && code --install-extension output\vscmanta.vsix