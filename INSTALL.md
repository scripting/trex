These are the steps I took to build Trex.

Setup new 1GB VPS with Ubuntu 13.04 x64

Point a domain name at it as well as a wildcard subdomain.

In these instructions I use the domain "example.com" and have logged into my server as root.  If you're logged in as another use you may need to use `sudo`.

```
apt-get update
apt-get upgrade
apt-get install vim zip build-essential automake autoconf git-core subversion libssl-dev libtool
reboot
cd /var
git clone git@github.com:scripting/trex.git
cd trex
git submodule init
git submodule update
./build.sh
export LD_LIBRARY_PATH=/var/trex/deps/usr/lib
cd build
make install
trex https://raw.github.com/scripting/trex/master/opml/trexBoot.opml
```

Site loads at http://andrewshell.example.com:8080/

If you'd like to run this more long term you can use the command:

`nohup trex https://raw.github.com/scripting/trex/master/opml/trexBoot.opml &`

and it will continue to run after you log out.

Notes:

Originally tried installing on Ubuntu 12.04 LTS x64 however got the error:
configure.ac:4: error: Autoconf version 2.69 or higher is required
