import os
import time
import sys
import subprocess
import shutil
import glob
from time import sleep,localtime

TMP_DIR = "/tmp/ex4sanity"
USERS_DIR = os.getcwd()

ROOTDIR = TMP_DIR + "/root"
MOUNTDIR = TMP_DIR + "/mount"

BLOCKNUM = 10
FNEW = 0.1
FOLD = 0.1

EXEFILE = "./CachingFileSystem"

RUNCMD = EXEFILE + " " + str(ROOTDIR) + " " + str(MOUNTDIR) + " " + str(BLOCKNUM) + " " + str(FNEW) + " " + str(FOLD)
RUNCMDXY = EXEFILE + " " + str(ROOTDIR) + " " + str(MOUNTDIR) + " "

runFuse = lambda x,y,z: os.system(RUNCMDXY + str(x) + " " + str(y) + " " + str(z))
UNFUSE = "fusermount -u " + MOUNTDIR

orgDir = os.getcwd()
GLOorgDir = os.getcwd()

tarball = "NOT_INIT"


def getExceededLines(path, maxColumn):
    exceededLines = 0
    with open(path, "r") as ins:
        for line in ins:
            if len(line.expandtabs(4)) > maxColumn:
                exceededLines += 1
    return exceededLines

def check_exceeding_line(filesInDir):
    
    #checking that all lines, in all files, don't exceed.
    for file in filesInDir:
        path = os.path.join(os.getcwd(), file)
        if os.path.isdir(path):
            continue
        exceededLines = getExceededLines(path, 85)
        if exceededLines != 0:
            print str(exceededLines) + "exceeding line(s) in file: " + file


def transFile():
    # Creating the tmp folder
    if not os.path.exists(TMP_DIR):
        os.makedirs(TMP_DIR)
    else:
        subprocess.call(["rm", "-rf", TMP_DIR+"/*"])

        
    # Extracting the files
    os.chdir(USERS_DIR)
    with open(os.devnull, "w") as f:
        subprocess.call(["tar", "-xvf", tarball, "-C",TMP_DIR], stdout = f)
    
    filesInDir = os.listdir(USERS_DIR)
    check_exceeding_line(filesInDir)

    os.chdir(orgDir)

def makeFileTest():
    dirN = os.getcwd()
    os.chdir(TMP_DIR)
    try:
        subprocess.check_output("make", shell=True) # Runs "make" w/o printing to the std
        res = not os.path.exists(EXEFILE)
    except:
        return False 

    if res:
        raw_input("Name is not good, Rename it to '" + str(EXEFILE) + "' and press 'Enter'")
        return True
    else:
        # print "Makefile is good"
        return True
    
    # Cleaning compilation files
    for oFile in glob.glob(TMP_DIR + "/*.o"):
        os.remove (oFile)

    os.chdir(dirN)
    
def clean():
    shutil.rmtree(TMP_DIR)

def run_command(command):
    process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    
    return process

def environmentSetup():
    orgDir = os.getcwd()
        
    if os.path.exists(MOUNTDIR) or os.path.exists(ROOTDIR):
        environmentTearDown()
    
    os.mkdir(MOUNTDIR)
    os.mkdir(ROOTDIR)
    
    os.chdir(ROOTDIR)
    
    # Creating files
    fileLst = ["basic"]
    lineNum = [4]
    for eachFile,fileLen in zip(fileLst,lineNum):
        fd = open(eachFile,'w')
        for i in xrange(fileLen):
            fd.write(str(i)+": I can C!\n")
        fd.close()
    
    os.chdir(orgDir)
    
def environmentTearDown():
    cleaned = False
    while(not cleaned):
        try:
            if os.path.exists(".filesystem.log"):
                os.remove(".filesystem.log")
            
            os.system(UNFUSE)
            
            if os.path.exists(MOUNTDIR):
                shutil.rmtree(MOUNTDIR)
            if os.path.exists(ROOTDIR):
                shutil.rmtree(ROOTDIR)
            
            cleaned = True
        except:
            print "Error: could not teardown."

def Test1():
    # Start
    environmentSetup()
    os.system(RUNCMD)
    orgDir = os.getcwd()
    os.chdir(MOUNTDIR)
    retVal = True
    
    
    fInF = os.listdir(os.getcwd())
    for eachFile in ["basic"]:
        if eachFile not in fInF:
            print eachFile," is not in the mount folder."
            retVal = "fail"
    
    # End
    os.chdir(orgDir)
    environmentTearDown()
    
    return retVal

README_OK = False

def exeTest(msg,testNum):
    global outFile
    global failedTest
    global README_OK
    
    if not msg == True:
        print "Error: " + msg
        testNum = msg.split(':')[0]
        logMsg = "\tERROR_" + str(testNum) + "\n"
        failedTest.append(int(testNum))
        
        outFile.write(logMsg)
    else:
	if README_OK:
        	print "Sanity check passed."
	else:
		print "Not ok"

def printNames():
    global outFile
    readmeFile = open("README","r")
    outFile.write(readmeFile.readline().strip()+":\n")
    readmeFile.close()

import re

BAD_LOGINS = 0
BAD_README = 1

# if the login(s) correctly extracted they and the no_appeal error are printed 
def test_README():
    usernames = None
    readme_filename = os.path.join(os.getcwd(), 'README')

    README_file_obj = open(readme_filename,'r')
    error_codes = set()

    # first line in README: logins
    line = README_file_obj.readline()
    p=re.compile('^(([\w\.]+)\s*)((\,\s?([\w\.]+))\s*)?\n$')
    m = p.match(line)
    if not m:
        README_file_obj.close()
        print("bad logins in README")
        return False

    # parse the second line: names and ids
    r = re.compile('^(\w+\s+)(\w+\s?)(\(\d+\)\s*)(,\s+(\w+\s+)(\w+\s?)(\(\d+\)\s*))?\n$')
    line = README_file_obj.readline()
    m = r.match(line)
    if not m:
        print("bad names and ids in README")
        return False

    # parse the third line: EX: 3
    r = re.compile('^(EX: 4)\n', re.IGNORECASE)
    line = README_file_obj.readline()
    m = r.match(line)
    if not m:
        print("no line Ex: 4 in README")
        return False

    # parse the 4th line: empty line
    r = re.compile('(\n)')
    line = README_file_obj.readline()
    m = r.match(line)
    if not m:
        print("4th line in README not empty")
        return False

    README_file_obj.close()
    return True


def main(givenName):
    global baseDir
    baseDir = os.getcwd()
    
    global tarball
    tarball = givenName
    global outFile
    outFile = open("results",'w');

    # Creating the tmp folder and moving all the files there
    transFile()
    
    # Making the files
    if makeFileTest() == False:
        # Erasing the tmpfile
        os.chdir (orgDir)
        clean()
        
        outFile.write('\tBAD_MAKEFILE\n')
        outFile.close()
        return
    
    global failedTest
    failedTest = []

    global README_OK
    README_OK = test_README()
    if not README_OK:
	print "README check failed."
    
    try:
        exeTest(Test1(),1) # Sanity check
        
    except Exception as e:
        sys.stderr.write(str(e))
        print "\nexeption!"
        # End
        os.chdir(orgDir)
        isClean = False
        while(not isClean):
            try:
                environmentTearDown()
                isClean = True
            except:
                sleep(1)
        
        outFile.write('\tEXEPTION\n')
        outFile.close()
    

    # Erasing the tmpfile
    os.chdir(orgDir)
    isClean = False
    while(not isClean):
        try:
            clean()
            isClean = True
        except:
            sleep(1)

    return failedTest

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print "Usage: python sanity.py <tarname>"
        
    else: 
        main(sys.argv[1])

