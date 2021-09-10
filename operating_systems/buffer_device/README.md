# Buffer device
This is a driver for a new device in the MINIX system. It simulates a buffer that acts like a queue when accessed by `read` or `write`. It also allows for the following ioctl operations:
- reset device (fill it with looping "xyz")
- insert a string to the end of the queue
- replace all occurences of one `char` with another one
- remove every third character (third, sixth, ninth etc.)
