#!/usr/bin/env python

import os
import re
import signal
import stat
import sys
import time
import ssl
import urllib.request
import shutil
import shlex
import subprocess

DEFAULT_IMAGE_PATH = '/home/root/bmc_image'

# Run bash command and print output to stdout
def run_command(command):
    proc = subprocess.Popen(shlex.split(command), shell=False, stdout=subprocess.PIPE)
    (out, err) = proc.communicate()

    if proc.returncode != 0:
        sys.exit(proc.returncode)

def validate_checksum(checksum):
    #check the md5 checksum
    command="md5sum " + DEFAULT_IMAGE_PATH
    proc = subprocess.Popen(shlex.split(command), shell=False, stdout=subprocess.PIPE)
    (out, err) = proc.communicate()

    if proc.returncode != 0:
        return False

    file_checksum=out.decode().split(' ')[0]
    print("calculated checksum: " + file_checksum)
    if checksum == file_checksum:
       print("checksum match")
       return True
    else:
       print("checksum not match")
       return False

# Install image
def install(url):
    """ Install image from local binary or URL"""
    ssl._create_default_https_context = ssl._create_unverified_context
    file_name=DEFAULT_IMAGE_PATH
    print("Downloading the image,  it will take a few minutes ...")
    with urllib.request.urlopen(url) as u, \
        open(file_name, 'wb') as f:
            f.write(u.read())

    """ download md5 checksum file """
    suffix_md5=".md5"
    url_md5=url + suffix_md5
    file_name_md5=file_name + suffix_md5
    fname, h = urllib.request.urlretrieve(url_md5, file_name_md5)

    """ read md5 checksum """
    try:
       with open (file_name_md5, "r") as checksum_file:
           checksum = checksum_file.readline().strip()
           print("checksum is: " + checksum)

    except:
       print("Could not read file: " + file_name_md5)
       sys.exit(1)

    checksum_file.close()

    run_command("sync")
    if validate_checksum(checksum) is False:
        print("checksum not match, exit")
        sys.exit(1)

    run_command("sleep 3") # wait 3 seconds after sync

    print("program the flash,  it will take a few minutes ...")
    run_command('/usr/local/bin/flash-upg' + ' 0 ' + DEFAULT_IMAGE_PATH)
    print("flash programmed,  rebooting BMC ...")
    run_command("reboot")

if __name__ == '__main__':
    if (len(sys.argv) == 3):
        if (sys.argv[2].lower() != "-y"):
            print("wrong option,  option '-y' to proceed")
            sys.exit(1)
    elif (len(sys.argv) == 2):
        print("Will program the flash, want to continue? (Y/N)?")
        answer = input()
        if answer.lower() != 'y':
            sys.exit(1)
    else:
        print("Invalid: Please provide image url with no option or url with option '-y'")
        sys.exit(1)

    install(sys.argv[1])
