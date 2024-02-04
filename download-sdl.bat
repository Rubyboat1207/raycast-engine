@echo off
setlocal
curl -OL https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-devel-2.30.0-VC.zip
if exist SDL2-devel-2.30.0-VC.zip (
   tar -xf SDL2-devel-2.30.0-VC.zip
   rm SDL2-devel-2.30.0-VC.zip
)else (
   echo "Failed to download ZIP archive from github. Check your internet connectivity or if the link is dead update with a 2.30.0 version of SDL2
)