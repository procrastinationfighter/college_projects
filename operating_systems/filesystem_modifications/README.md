 # Filesystem modifications
 The patch modifies the mfs filesystem by creating following errors:
 1. Every third byte written to a file will be modified by adding 1 (mod 256) to it. Example: 
```
$ ls
$ echo '1234567' > ./f1
$ cat ./f1
1244577
$ echo '1234567' > ./f2
$ cat ./f2
1244577
```
2. Every third `chmod` operation will set invalid value for the write permission for the others group. Example:
```
# ls -l
total 16
-rwxrwxr-x  1 root  operator  8 Apr 26 17:02 f1
# chmod 777 ./f1
# ls -l
total 16
-rwxrwxrwx  1 root  operator  8 Apr 26 17:02 f1
# chmod 777 ./f1
# ls -l
total 16
-rwxrwxrwx  1 root  operator  8 Apr 26 17:02 f1
# chmod 777 ./f1
# ls -l
total 16
-rwxrwxr-x  1 root  operator  8 Apr 26 17:02 f1
```
3. All removed files will be moved to a subdirectory `debug` if such directory exists in current directory.
