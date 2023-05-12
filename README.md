Currently this synth is only tested on MacOS. It might be possible to get it to build on Linux with little modification
but Windows is unsupported.

The build configuration also assumes that there is a working PluginHost target for testing the synth.

Use CMake to generate the build configs for XCode.

```
git clone https://github.com/stroebel/jx11.git
cd jx11
git submodule update --init --recursive
cmake -Bbuild
cmake -Bbuild -G Xcode
```
