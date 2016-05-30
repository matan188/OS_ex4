import unittest
import os
import time
import shutil
import fcntl
import re
import subprocess

BASE_ROOT_DIR = 'tests_rootdir'
BASE_MOUNT_DIR = 'tests_mountdir'
LOG_FILE_NAME = '.filesystem.log'
USAGE_ERROR = b'Usage: CachingFileSystem rootdir mountdir numberOfBlocks fOld fNew\n'
IOCTL_LOG = 'ioctl'
basic_str = b'abcd'
logCommandRegex = re.compile('\d+ ([a-z]*)')
testCounter = 0

def createNormalDir(dirPath):
	os.makedirs(dirPath)
	a = open(os.path.join(dirPath,'a.txt'),'wb')
	a.write(basic_str)
	a.close()
	b = open(os.path.join(dirPath,'b.txt'),'wb')
	b.write(basic_str)
	b.close()

def initRootDir(rootdir):  
	createNormalDir(rootdir)
	createNormalDir(os.path.join(rootdir, 'dir1'))  
	createNormalDir(os.path.join(rootdir, 'dir2'))
	createNormalDir(os.path.join(rootdir, 'dir3'))
	createNormalDir(os.path.join(rootdir, 'dir11'))
	createNormalDir(os.path.join(rootdir, 'dir3', 'dir1'))
	os.makedirs(os.path.join(rootdir, 'dir1','dir4'))
	b = open(os.path.join(rootdir,'dir2', LOG_FILE_NAME),'wb')
	b.write(basic_str)
	b.close()

class TestInvalidArgsCachingFileSystem(unittest.TestCase):

	def setUp(self):
		self.unmountfuse = False
		self.timestamp = int(time.time())
		global testCounter
		testCounter += 1
		self.rootdir = '/tmp/tests_caching_rootdir_%s_%s' % (testCounter,self.timestamp)
		self.mountdir = '/tmp/tests_mountdir_%s_%s' % (testCounter,self.timestamp)
		os.makedirs(self.rootdir)
		os.makedirs(self.mountdir)


	def tearDown(self):
		try :
			shutil.rmtree(self.rootdir)
			os.rmdir(self.mountdir)
		except OSError:
			pass

	def test_new_empty_section(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir, '5', '0.0', '0.4']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_old_empty_section(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir, '5', '0.4', '0.0']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_old_and_new_more_than_cache_size_section(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir, '5', '0.6', '0.6']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_new_empty_section_despite_positive_percent(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir, '5', '0.05', '0.4']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_old_empty_section_despite_positive_percent(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir, '5', '0.4', '0.05']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_not_exist_root_dir(self):
		args = ['./CachingFileSystem', self.rootdir+'_not_exist', self.mountdir, '5', '0.4', '0.05']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_not_exist_mount_dir(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir+'_not_exist', '5', '0.4', '0.05']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_cache_size_is_0(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir, '0', '0.4', '0.05']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

	def test_cache_size_is_negative(self):
		args = ['./CachingFileSystem', self.rootdir, self.mountdir, '-5', '0.4', '0.05']
		with subprocess.Popen(args, stdout=subprocess.PIPE) as p:
			p.wait(timeout=1)
			self.assertNotEqual(p.returncode, 0)
			self.assertEqual(p.stdout.read(), USAGE_ERROR)

class TestChachingFileSystem(unittest.TestCase):
	

	def setUp(self):
		self.timestamp = int(time.time())
		global testCounter
		testCounter += 1
		self.rootdir = '/tmp/tests_caching_rootdir_%s_%s' % (testCounter,self.timestamp)
		self.mountdir = '/tmp/tests_mountdir_%s_%s' % (testCounter,self.timestamp)
		initRootDir(self.rootdir)
		os.makedirs(self.mountdir)
		self.blocksize = os.stat(self.rootdir).st_size
		large = open(os.path.join(self.rootdir,'large.txt'),'w')
		for i in range(1,10):
			large.write(str(i)*self.blocksize)   
		large.write(basic_str.decode('utf-8'))
		large.close()
		os.system('touch ' + os.path.join(self.rootdir, LOG_FILE_NAME))
		os.system('./CachingFileSystem %s %s 5 0.4 0.4' % (self.rootdir, self.mountdir))
		self.log = open(os.path.join(self.rootdir, LOG_FILE_NAME),'r')
		self.fd = os.open(os.path.join(self.mountdir,'a.txt'),os.O_RDONLY | os.O_DIRECT)

	def tearDown(self):
		os.close(self.fd)
		os.system('fusermount -u %s' % self.mountdir)
		self.log.close()
		shutil.rmtree(self.rootdir)
		os.rmdir(self.mountdir)   

	def logCacheStatus(self):
		fd = os.open(os.path.join(self.mountdir, 'a.txt'),  os.O_RDONLY)
		# fd = os.open(mountdir,  os.O_RDONLY)
		fcntl.ioctl(fd, 0)
		os.close(fd)
		loglines = self.log.readlines()
		for index, line in enumerate(loglines):
			m = logCommandRegex.match(line)
			if m != None and m.group(1) == IOCTL_LOG:
				break
		startIndex = index+1
		for index, line in enumerate(loglines[startIndex:]):
			m = logCommandRegex.match(line)
			if m != None:
				break
		return loglines[startIndex:startIndex+index] 

	def test_basic_open_success(self):
		self.assertTrue(self.fd > 0)

	def test_open_not_exist_file(self):
		with self.assertRaises(OSError):
			fd = os.open(os.path.join(self.mountdir,'notexist'), os.O_RDONLY)

	def test_open_for_write(self):
		with self.assertRaises(OSError):
			fd = os.open(os.path.join(self.mountdir,'b.txt'), os.O_RDWR)
		with self.assertRaises(OSError):
			fd = os.open(os.path.join(self.mountdir,'b.txt'), os.O_WRONLY)

	def test_open_file_twice(self):
		fd = os.open(os.path.join(self.mountdir,'a.txt'), os.O_RDONLY)
		data = os.read(fd,len(basic_str))
		self.assertEqual(data,basic_str)
		os.close(fd)
		data = os.read(self.fd,len(basic_str))
		self.assertEqual(data,basic_str)

	def test_open_log_not_in_mount_dir(self):
		fd = os.open(os.path.join(self.mountdir, 'dir2', LOG_FILE_NAME), os.O_RDONLY)
		os.close(fd)

	def test_open_log_in_mount_dir(self):
		with self.assertRaises(OSError):
			fd = os.open(os.path.join(self.mountdir, LOG_FILE_NAME), os.O_RDONLY)
			os.close(fd)

	def test_basic_read(self):
		data = os.read(self.fd, len(basic_str))
		self.assertEqual(data, basic_str)

	def test_read_data_twice(self):
		data = os.pread(self.fd, len(basic_str),0)
		self.assertEqual(data, basic_str)
		data = os.pread(self.fd, len(basic_str),0)
		self.assertEqual(data, basic_str)

	def test_read_with_offset(self):
		data = os.pread(self.fd, len(basic_str) -1,1)
		self.assertEqual(data, basic_str[1:])

	def test_read_with_out_of_size(self):
		data = os.pread(self.fd, len(basic_str) +1, 0)
		self.assertEqual(data, basic_str)

	def test_read_with_out_of_size_with_offset(self):
		data = os.pread(self.fd, len(basic_str), 1)
		self.assertEqual(data, basic_str[1:])

	def test_complex_read_with_offset_and_size_larger_than_blocksize(self):
		fd = os.open(os.path.join(self.mountdir, 'large.txt'), os.O_RDONLY)
		data = os.pread(fd, len(basic_str)+self.blocksize - 1, self.blocksize*8)
		os.close(fd)
		self.assertEqual(data[:self.blocksize],b'9'*self.blocksize)
		self.assertEqual(data[self.blocksize:],basic_str[:-1])

	def test_basic_read_less_than_file_size(self):
		data = os.read(self.fd, 2)
		self.assertEqual(data, basic_str[:2])

	def test_basic_listdir(self):
		listdir = os.listdir(os.path.join(self.mountdir,'dir1'))
		self.assertIn('a.txt', listdir)
		self.assertIn('b.txt', listdir)
		self.assertIn('dir4', listdir)
		self.assertEqual(len(listdir), 3)

	def test_listdir_dont_show_log(self):
		listdir = os.listdir(self.mountdir)
		self.assertIn('a.txt', listdir)
		self.assertIn('b.txt', listdir)
		self.assertIn('large.txt', listdir)
		self.assertIn('dir1', listdir)
		self.assertIn('dir2', listdir)
		self.assertIn('dir3', listdir)
		self.assertIn('dir11', listdir)
		self.assertNotIn(LOG_FILE_NAME, listdir)
		self.assertEqual(len(listdir), 7)

	def test_listdir_show_log_not_in_mount_dir(self):
		listdir = os.listdir(os.path.join(self.mountdir, 'dir2'))
		self.assertIn('a.txt', listdir)
		self.assertIn('b.txt', listdir)
		self.assertIn(LOG_FILE_NAME, listdir)
		self.assertEqual(len(listdir), 3)


	def test_basic_rename(self):
		newPath = os.path.join(self.mountdir,'dir3','c.txt')
		origPath = os.path.join(self.mountdir,'dir3','a.txt')
		os.rename(origPath, newPath)
		fd = os.open(newPath, os.O_RDONLY)
		data = os.read(fd,len(basic_str))
		os.close(fd)
		self.assertEqual(data,basic_str)

	def test_basic_rename_dir(self):
		origPath = os.path.join(self.mountdir,'dir3','dir1')
		newPath = os.path.join(self.mountdir, 'dir3','dir2')
		os.rename(origPath, newPath)
		fd = os.open(os.path.join(newPath, 'a.txt'), os.O_RDONLY)
		data = os.read(fd,len(basic_str))
		os.close(fd)
		self.assertEqual(data,basic_str)	

	def test_caching_start_empty(self):
		cacheLog = self.logCacheStatus()
		self.assertEqual(len(cacheLog), 0)	

	def test_caching_add_block_after_read(self):
		data = os.read(self.fd, len(basic_str))
		self.assertEqual(data, basic_str)
		cacheLog = self.logCacheStatus()
		self.assertEqual(cacheLog[0], 'a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)	

	def test_caching_add_2_block_after_read(self):
		large = os.open(os.path.join(self.mountdir,'large.txt'), os.O_RDONLY)
		os.pread(large, 2, 0)
		os.pread(self.fd, 2, 0)
		os.close(large)
		cacheLog = self.logCacheStatus()
		print('debug')
		for l in cacheLog:
			print(l)
		self.assertEqual(len(cacheLog), 2)
		self.assertEqual(cacheLog[0], 'large.txt 1 1\n')
		self.assertEqual(cacheLog[1], 'a.txt 1 1\n')

	def test_full_cache_doesnt_increase(self):
		large = os.open(os.path.join(self.mountdir,'large.txt'), os.O_RDONLY)
		data = os.pread(large, self.blocksize, 0)
		data = os.pread(large, self.blocksize, self.blocksize)
		data = os.pread(large, self.blocksize, self.blocksize*2)
		data = os.pread(large, self.blocksize, self.blocksize*3)
		data = os.pread(large, self.blocksize, self.blocksize*4)
		data = os.pread(large, self.blocksize, self.blocksize*5)
		os.close(large)
		cacheLog = self.logCacheStatus()
		self.assertEqual(len(cacheLog), 5)
		self.assertEqual(cacheLog[0], 'large.txt 2 1\n')
		self.assertEqual(cacheLog[1], 'large.txt 3 1\n')
		self.assertEqual(cacheLog[2], 'large.txt 4 1\n')
		self.assertEqual(cacheLog[3], 'large.txt 5 1\n')
		self.assertEqual(cacheLog[4], 'large.txt 6 1\n')

	def test_caching_evicted_not_in_middle_or_new(self):
		large = os.open(os.path.join(self.mountdir,'large.txt'), os.O_RDONLY)
		data = os.pread(large, self.blocksize, 0)
		data = os.pread(large, self.blocksize, self.blocksize)
		data = os.pread(large, self.blocksize, self.blocksize*2)
		data = os.pread(large, self.blocksize, 0)
		data = os.pread(large, self.blocksize, self.blocksize)
		data = os.pread(large, self.blocksize, self.blocksize*2)
		data = os.pread(large, self.blocksize, self.blocksize*3)
		data = os.pread(large, self.blocksize, self.blocksize*4)
		data = os.pread(large, self.blocksize, self.blocksize*5)
		os.close(large)
		cacheLog = self.logCacheStatus()
		self.assertEqual(len(cacheLog), 5)
		self.assertEqual(cacheLog[0], 'large.txt 2 2\n')
		self.assertEqual(cacheLog[1], 'large.txt 3 2\n')
		self.assertEqual(cacheLog[2], 'large.txt 4 1\n')
		self.assertEqual(cacheLog[3], 'large.txt 5 1\n')
		self.assertEqual(cacheLog[4], 'large.txt 6 1\n')

	def test_caching_add_block_after_read_whole_block_with_offset_of_whole_block(self):
		large = os.open(os.path.join(self.mountdir,'large.txt'), os.O_RDONLY)
		data = os.pread(large, self.blocksize, self.blocksize)
		data = os.pread(large, self.blocksize, self.blocksize*2)
		os.close(large)
		cacheLog = self.logCacheStatus()
		self.assertEqual(len(cacheLog), 2)
		self.assertEqual(cacheLog[0], 'large.txt 2 1\n')
		self.assertEqual(cacheLog[1], 'large.txt 3 1\n')

	def test_caching_reference_counter_doent_increased_in_new_section(self):
		data = os.pread(self.fd, len(basic_str), 0)
		self.assertEqual(data, basic_str)
		cacheLog = self.logCacheStatus()
		self.assertEqual(cacheLog[0], 'a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)
		data = os.pread(self.fd, len(basic_str), 0)	
		cacheLog = self.logCacheStatus()
		self.assertEqual(cacheLog[0], 'a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)

	def test_caching_reference_counter_increased_in_middle_section(self):
		data = os.pread(self.fd, len(basic_str), 0)
		self.assertEqual(data, basic_str)
		cacheLog = self.logCacheStatus()
		self.assertEqual(cacheLog[0], 'a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)
		large = os.open(os.path.join(self.mountdir,'large.txt'), os.O_RDONLY)
		os.pread(large, self.blocksize, 0)
		os.pread(large, self.blocksize, self.blocksize)	
		data = os.pread(self.fd, len(basic_str), 0)	
		cacheLog = self.logCacheStatus()
		os.close(large)
		self.assertEqual(len(cacheLog), 3)
		self.assertEqual(cacheLog[0], 'large.txt 1 1\n')
		self.assertEqual(cacheLog[1], 'large.txt 2 1\n')
		self.assertEqual(cacheLog[2], 'a.txt 1 2\n')

	def test_cache_not_clean_on_file_close(self):
		origPath = os.path.join(self.mountdir,'dir3','a.txt')
		fd = os.open(origPath, os.O_RDONLY)
		data = os.pread(fd, len(basic_str), 0)
		cacheLog = self.logCacheStatus()
		self.assertEqual(cacheLog[0], 'dir3/a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)
		os.close(fd)
		cacheLog = self.logCacheStatus()
		self.assertEqual(cacheLog[0], 'dir3/a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)

	def test_caching_work_on_file_rename(self):
		origPath = os.path.join(self.mountdir,'dir3','a.txt')
		newPath = os.path.join(self.mountdir,'dir3','c.txt')
		fd = os.open(origPath, os.O_RDONLY)
		data = os.pread(fd, len(basic_str), 0)
		os.close(fd)
		os.rename(origPath, newPath)
		cacheLog = self.logCacheStatus()
		self.assertEqual(cacheLog[0], 'dir3/c.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)

	def test_caching_work_on_dir_rename(self):
		origPath = os.path.join(self.mountdir,'dir3','dir1')
		newPath = os.path.join(self.mountdir, 'dir3','dir2')
		fd = os.open(os.path.join(origPath,'a.txt'), os.O_RDONLY)
		data = os.pread(fd, len(basic_str), 0)
		os.rename(origPath, newPath)
		cacheLog = self.logCacheStatus()
		os.close(fd)
		self.assertEqual(cacheLog[0], 'dir3/dir2/a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)

	def test_caching_work_on_dir_rename_when_another_dir_start_with_this_name(self):
		origPath = os.path.join(self.mountdir, 'dir1')
		newPath = os.path.join(self.mountdir, 'dir12')
		fd = os.open(os.path.join(self.mountdir, 'dir11','a.txt'), os.O_RDONLY)
		data = os.pread(fd, len(basic_str), 0)
		os.rename(origPath, newPath)
		cacheLog = self.logCacheStatus()
		os.close(fd)
		self.assertEqual(cacheLog[0], 'dir11/a.txt 1 1\n')
		self.assertEqual(len(cacheLog), 1)

	def test_rename_log_in_mount_dir(self):
		origPath = os.path.join(self.mountdir, LOG_FILE_NAME)
		newPath = os.path.join(self.mountdir, 'dir3', 'abcd'+LOG_FILE_NAME)
		with self.assertRaises(OSError):
			os.rename(origPath, newPath)

	def test_rename_log_not_in_mount_dir(self):
		origPath = os.path.join(self.mountdir, 'dir2', LOG_FILE_NAME)
		newPath = os.path.join(self.mountdir, 'dir3', 'abcd'+LOG_FILE_NAME)
		os.rename(origPath, newPath)

	def test_getattr_log_in_mount_dir(self):
		with self.assertRaises(OSError):
			os.stat(os.path.join(self.mountdir, LOG_FILE_NAME))

	def test_getattr_log_not_in_mount_dir(self):
		os.stat(os.path.join(self.mountdir, 'dir2', LOG_FILE_NAME))

	def test_access_log_in_mount_dir(self):
		self.assertFalse(os.access(os.path.join(self.mountdir, LOG_FILE_NAME), os.R_OK))

	def test_access_log_not_in_mount_dir(self):
		self.assertTrue(os.access(os.path.join(self.mountdir, 'dir2', LOG_FILE_NAME), os.R_OK))	


if __name__ == '__main__':
	unittest.main()
	# todo add test that reference count inc in old section
	# todo add test that tests the old removing
