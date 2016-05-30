cat /tmp/mountdir/folder1/file
cat /tmp/mountdir/folder1/file
cat /tmp/mountdir/folder2/file
cat /tmp/mountdir/folder2/file


mv /tmp/mountdir/folder2/ /tmp/mountdir/folder3
mv /tmp/mountdir/folder1/ /tmp/mountdir/folder2
cat /tmp/mountdir/folder2/file
cat /tmp/rootdir/folder2/file

mv /tmp/mountdir/folder2 /tmp/mountdir/folder1
mv /tmp/mountdir/folder3 /tmp/mountdir/folder2
