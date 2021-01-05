# Coventina-Android
Android port of voximorg/Coventina (WIP)

# Usage
```shell
git clone https://github.com/voximorg/Coventina-Android.git
git submodule update --init
```

Import this directory from Android Studio as a project.

There may be some more platform specific steps or details to be worked out from there.

# Work in Progress
I have modified Coventina through the use of dark magic and some serious hackery to work on Android without SDL.

All of the source files have been concatenated into one at cpp/Coventina/coventina.cpp. (through the use of cat)

I have surgically removed a lot of code. All the audio code is gone and this means that there is no audio output currently. All code leveraging SDL, SDL_mixer and SDL_image is gone. SDL_image is replaced by stb_image.h and the rest of the SDL code is either unecessary on Android or handled by callbacks from Java code through the Java Native Interface (JNI). 

GLM is in use (just like original Coventina) and packaged as a submodule under cpp/

This is meant as an initial demo. I did what I needed to do to get something that works and is presentable, but not much more. Treat this as a jumping off point.
