# Negate exit
This patch adds a new function to the unistd library. ```int negateexit(int negate)``` causes current process and its children (created after the call) 
to change their exit calls. If ```negate``` is equal to zero, the calls will be unchanged. Otherwise, the exit calls returning ```0``` will be changed to ```1``` 
and all other exit codes will be changed to ```0```. 

The only changed values are the exit codes related to the call PM_EXIT in the PM server.
