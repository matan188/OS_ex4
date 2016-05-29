import os
fd=os.open('/tmp/mountdir/file02', os.O_RDONLY)
os.lseek(fd, 3958, os.SEEK_SET)
ret=os.read(fd, 2344)
print ret
