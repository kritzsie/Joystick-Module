Joystick-Module
===============

A joystick module for GMod, originally by NightEagle (http://facepunch.com/member.php?u=2853).

Original Facepunch thread: http://www.facepunch.com/showthread.php?t=403669

This version adds support for non-Windows platforms via SDL. It should compile with:
```
g++ -Wall -fPIC -shared -lSDL -lpthread -o "../lua/bin/gmcl_joystick.so" "main.cpp"
```

Installation
===============

Merge the 'addons' and 'lua' folders from this folder into your 'garrysmod' folder.
