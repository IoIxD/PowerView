# PowerView (WIP)

Server/program that seamlessly integrates programs running on an X11 server (see: modern Mac, Linux) with an old PowerPC or 68k Macintosh. It's similar in principle to [winapps](https://github.com/Fmstrat/winapps), except we use our own simpler protocol as opposed to RDP (and also we assume you're not gonna be running a VM).

The client (written in C++) is only supported on Classic Mac OS (though it can be run on Linux with limited features). I do not plan on targetting old Windows; I don't find it fun to develop for (unless I'm using Qt which sucks to get working), and besides I currently do not have an old Windows setup to test PRs on.

The server (written in Rust) is currently only supported on platforms that use X. People are invited to port it to Windows, but it's personally not a priority for me.

# Building

**Server:** Install Rust and XVFB, then go into the `server` directory and run `cargo build --release` (or without `--release` for debug mode). The resulting binary will be in the `target/release` (or `target/debug`) folder. You can also swap out `build` for `run` to instantly run the binary.

**Client:** 
- Install whatever your OS/distro needs for building C/C++ packages