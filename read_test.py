import os
fd=os.open('/tmp/mountdir/file02', os.O_RDONLY)
os.lseek(fd, 4096, os.SEEK_SET)
ret=os.read(fd, 4096*2)
print(ret)
