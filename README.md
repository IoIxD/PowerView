# PowerView (WIP)

Server/program that seamlessly integrates programs running on an X11 server (see: modern Mac, Linux) with an old PowerPC or 68k Macintosh. It's similar in principle to [winapps](https://github.com/Fmstrat/winapps), except we use our own simpler protocol as opposed to RDP (and also we assume you're not gonna be running a VM).

The *client* (written in C++) is only fully supported on Classic Mac OS (it can also be compiled for any platform with OpenGL support, albiet with limited features). I do not plan on targetting old Windows as I don't find it fun to develop for (unless I'm using Qt which sucks to get working on older Windows), but PRs are open for anybody who wants to try.

The *server* (written in Rust) is currently only supported on platforms that use X. Currently, only executables are provided for Linux and Intel Macs; those on Silicon will need to compile the server using the below instructions. People are invited to port it to Windows, but it's personally not a priority for me.

# Building

**Server:** Install Rust and XVFB, then go into the `server` directory and run `cargo build --release` (or without `--release` for debug mode). The resulting binary will be in the `target/release` (or `target/debug`) folder. You can also swap out `build` for `run` to instantly run the binary.

**Client:** 
- Install whatever your OS/distro needs for building C/C++ packages (i.e. XCode on Mac, `build-essential` on Ubuntu)
- Go into the `client` directory and make a `build` directory. 
- (For Linux) Do `cmake ..` then `make -j$(nproc)`
- (For Classic Mac) Remind me to put something here. 