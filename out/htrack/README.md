# HeadTrack

Headtrack is a lightweight head tracking plugin for X-Plane. It listens on port 4242 for UDP messages from apps like [SmoothTrack][smoothtrack] and uses that to move your view about in X-Plane.

## Building

This will only work using a posix-style environment (which means MingW on windows). There are no other dependencies, just generate the CMake build files and hit `make`.

## Caveats

The whole plugin, really. The only reason this exists is because I was tired of fighting OpenTrack to work on linux. At the moment it's *rough* (it was written in half a day). Things that might happen down the road:

* network settings (!). At the moment, the server listens on `0.0.0.0`, port `4242`, which is not ideal.
* settings window in X-Plane to let users set sensitivity, smoothing, etc;
* config file support (so each plane can have its own configuration);

This is essentially a toy project, and I can't make any guarantees any of these will happen! If you have ideas or bugs, please open an issue or contact me, or feel free to fork away and make your own version :)

[smoothtrack]: https://smoothtrack.app/

