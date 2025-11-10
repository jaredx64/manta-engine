@echo off

mkdir extension
xcopy syntax       extension\syntax /i
xcopy extension.js extension
xcopy package.json extension

7z a -tzip -sdel vscmanta.vsix extension
code --install-extension vscmanta.vsix