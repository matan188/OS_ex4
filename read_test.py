import os
fd=os.open('/tmp/mountdir/file02', os.O_RDONLY)
os.lseek(fd, 0, os.SEEK_SET)
ret=os.read(fd, 5000)
print ret
