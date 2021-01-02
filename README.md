# xpanic

This xPanic remake was originally maintained by [@kurosio](https://github.com/kurosio) but the source is over 2 years old and does not compile. This repository is an effort to fix, maintain, and revitalize the game mode.

NOTE: The source in this repository is based on a 2016 commit that *did* compile.

### Building from source

1. Download [bam v0.4.0](https://github.com/matricks/bam/releases/tag/v0.4.0)
2. Build bam using the appropriate build script for your operating system (only tested with UNIX)
2. Create an `accounts/` directory (this step is extremely important!)
3. Create an `autoexec_server.cfg` file. An example file is provided as a reference.
**DO NOT JUST COPY AND PASTE THE CONTENTS, CARE MUST BE TAKEN TO SET SECURE VALUES + DEFAULTS**
2. Run `../bam/bam server_release` to compile the server binary
