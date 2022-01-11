#!/usr/bin/python
"""
Module for reimaging powershelves
facilitates downloading, validating and flashing image
"""
from datetime import datetime
import urllib.request
import subprocess
import hashlib
import json
import ssl
import sys
import os
import os.path

CONFIG_FILE = '/home/root/reimage.json'
LOG_FILE = '/mnt/data/reimage_log'
FREE_MEM_REQUIRED = 35000

def execute_command(command, shell=False):
    """ execute shell command """
    proc_args = command.strip().split()
    proc = subprocess.Popen(proc_args,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            shell=shell)
    with proc:
        stdout, stderr = proc.communicate()
        returncode = proc.returncode
        return (stdout, stderr, returncode)

def log(message):
    """ log a timestamp and message into a log file """
    try:
        with open(LOG_FILE,'a+') as reimage_log:
            now = datetime.now()
            dt_string = now.strftime("%m/%d/%Y %H:%M:%S")
            log_str = dt_string + " " + message
            print(log_str)
            reimage_log.write(log_str + "\n")
    except:
        return

def get_free_mem_bytes():
    """ return free memory in bytes """
    try:
        meminfo = open("/proc/meminfo")
    except:
        log("Could not read file: /proc/meminfo, aborting")
    with meminfo:
        for line in meminfo:
            if "MemFree" in line:
                line_list = line.split()
                free_memory = int(line_list[1])
                return free_memory
    return 0

def md5(fname):
    """ calculate md5sum of a file """
    hash_md5 = hashlib.md5()
    try:
        file = open(fname, "rb")
    except:
        log("Could not read file: " + fname +", aborting")
        sys.exit(1)

    with file:
        for chunk in iter(lambda: file.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

def validate_checksum(file_name):
    """ validate md5 checksum of an image file vs provided md5 """
    image_md5 = "%s.md5" % (file_name)
    # read checksum
    try:
        with open (image_md5, "r") as checksum_file:
            checksum = checksum_file.readline().strip()
    except:
        log("Could not read file: " + image_md5 +", aborting")
        sys.exit(1)

    file_md5 = md5(file_name)
    if checksum != file_md5:
        log("Checksum not matching, aborting")
        sys.exit(1)

def download_image(image, image_folder):
    """ download image and md5 checksum file """
    ssl._create_default_https_context = ssl._create_unverified_context
    image_md5 = "%s.md5" % (image)
    image_url = os.path.join(image_folder, image, image)
    image_md5_url = os.path.join(image_folder, image, image_md5)

    if os.path.exists(image):
        try:
            os.remove(image)
        except OSError:
            log("File " + image + " can not be removed")
            sys.exit(1)

    if os.path.exists(image_md5):
        try:
            os.remove(image_md5)
        except OSError:
            log("File " + image_md5 + " can not be removed")
            sys.exit(1)

    # check free memory and if we can fit image
    free_mem = get_free_mem_bytes()
    if free_mem < FREE_MEM_REQUIRED:
        log("No free memory (less than 35MB)")
        sys.exit(1)

    try:
        urllib.request.urlretrieve(image_url, image)
    except Exception:
        log("Exception downloading " + image +", aborting")
        sys.exit(1)

    if not os.path.exists(image):
        log("Could not download image file: " + image +", aborting")
        sys.exit(1)

    try:
        urllib.request.urlretrieve(image_md5_url, image_md5)
    except Exception:
        log("Exception downloading " + image_md5 +", aborting")
        sys.exit(1)

    if not os.path.exists(image_md5):
        log("Could not download image file: " + image_md5 +", aborting")
        sys.exit(1)

def get_platform():
    """ identify and return type of powershelf """
    stdout, _, returncode = execute_command('/usr/local/bin/get_sw_version.sh')
    if returncode == 0:
        if 'Delta' in str(stdout):
            return 'Delta'
        if 'Lighting' in str(stdout):
            return 'Lightning'
    return None

def image_platform(image):
    """ identify and return which platform image is suitable for """
    if 'delta' in image:
        return 'Delta'
    if 'lightning' in image:
        return 'Lightning'
    return None

def validate_platform(image):
    """ validate that image is suitable for the powershelf """
    platform = get_platform()
    img_platform = image_platform(image)

    if platform is None or img_platform is None:
        log('Could not determine if image is matching the platform, aborting')
        delete_config_file(CONFIG_FILE)
        sys.exit(1)

    if platform != img_platform:
        log('Image type does not match the platform, aborting')
        delete_config_file(CONFIG_FILE)
        sys.exit(1)

def read_config(config_file):
    """ read configuration file """
    try:
        with open(config_file, "r") as file:
            data = json.load(file)
            image = data['image']
            image_folder = data.get('image_folder_url', None)
            flash_number = data.get('flash_num', 0)
            return image, image_folder, flash_number
    except FileNotFoundError:
        print('Configuration not present')
        sys.exit(1)
    except KeyError:
        print('Required data missing in configuration file')
        sys.exit(1)
    except:
        print('Could not read reimage configuration file')
        sys.exit(1)

def delete_config_file(file_path):
    """ delete configuration file """
    try:
        os.remove(file_path)
    except:
        print(" Error while deleting file : ", file_path)

# check if config file exists
if not os.path.isfile(CONFIG_FILE):
    sys.exit(0)

# read json reimage configuration file
image_file, image_folder_url, flash_num = read_config(CONFIG_FILE)

# check if image is suitable for the platform
validate_platform(image_file)

# download the image if not local installation
if image_folder_url:
    download_image(image_file, image_folder_url)

# validate image checksum
validate_checksum(image_file)

# flash the image
stdout_, stderr_, returncode_ = execute_command('/usr/local/bin/flash-upg' + \
                                ' ' + str(flash_num) + ' ' + image_file)
if returncode_ == 0:
    log("success: flashed: " + image_file)
    execute_command("reboot")
else:
    log("failure: flashing failed: " + image_file)

