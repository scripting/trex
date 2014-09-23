Installation on Linux
=====================

These are the steps I took to build Trex.  This has been tested on Ubuntu 13.04 x64

Notes:

Originally tried installing on Ubuntu 12.04 LTS x64 however got the error:
configure.ac:4: error: Autoconf version 2.69 or higher is required

I have logged into my server as root.  If you're logged in as another use you may need to use `sudo`.

Install Prerequisites:

    apt-get update
    apt-get upgrade
    apt-get install vim zip build-essential automake autoconf git-core subversion libssl-dev libtool
    reboot


Build and install Trex:

    cd /var
    git clone git@github.com:scripting/trex.git
    cd trex
    git submodule init
    git submodule update
    ./build.sh

Trex will tell you to set the library path location.  Note this for later when trex is run.  Install trex:

    cd build
    make install


Installation on Mac OS X
========================

You will need to install some prerequisite packages followed by installing some dependent packages required to compile the code.  

1. Install git from [Git for Mac Download](http://git-scm.com/download/mac)

2. Install XCode (this is required as the version of v8 included as part of the referenced git submodule can only be built with XCode, later fixes were included to allow v8 to be built with the command line tools).

3. Now build the prerequisite tools required to compile trex using Homebrew:

    `brew install autoconf automake libtool openssl libxml2`

4. Compile the trex code.  Follow the compilation instructions for Linux above. 
When setting the library path below change `export LD_LIBRARY_PATH` to 
`export DYLD_LIBRARY_PATH`

Running Trex
============

Ensure trex can find the libraries created during build (if installed under /var/trex) - modify as necessary:

    export LD_LIBRARY_PATH=/var/trex/deps/usr/lib 

or on Mac: 
    
    export DYLD_LIBRARY_PATH=/var/trex/deps/usr/lib 
    
Run trex as follows:
    trex https://raw.github.com/scripting/trex/master/opml/trexBoot.opml

Site loads at http://localhost:8080/

If you'd like to run this more long term you can use the command:

    nohup trex https://raw.github.com/scripting/trex/master/opml/trexBoot.opml &

and it will continue to run after you log out.

Test trex:

    curl -H "Host: dave.smallpict.com" http://localhost:8080

The HTML for <http://scripting.com> should be displayed

Point a domain name at it as well as a wildcard subdomain.  In these instructions I use the domain "example.com".  

    http://andrewshell.example.com:8080

In order to configure to configure to serve your own pages the worldoutline file bundled with Trex will need to be updated.

