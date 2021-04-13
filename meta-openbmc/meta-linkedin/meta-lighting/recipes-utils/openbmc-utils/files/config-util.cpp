/*
 * config-util
 *
 * Copyright 2019-present LinkedIn. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <shadow.h>
#include <syslog.h>
#include <nlohmann/json.hpp>
#include <experimental/filesystem>

static constexpr const char *shadowFilePath = "/etc/shadow";
static constexpr const char *sshFolderPath = "/home/root/.ssh/";

static constexpr const char *hostConfigPath = "/mnt/data/host_config.json";
static constexpr const char *authorizedKeysPersistentPath = "/mnt/data/authorized_keys";
static constexpr const char *authorizedKeysFinalPath = "/home/root/.ssh/authorized_keys";

using json = nlohmann::json;

int changeUserPassword(std::string &inputString, std::string user, std::string newPassword)
{
    std::string toFind = user + ":";
    // shadow file contains root password hash. It starts after the "root:"
    // lets find its position
    size_t pos = inputString.find(toFind);
    if (pos == std::string::npos)
    {
        return -1;
    }

    size_t startPos = pos + toFind.size();
    // find position of the delimiting colon position
    size_t endPos =inputString.find(':', startPos);
    if (endPos == std::string::npos)
    {
        return -1;
    }

    // replace string between above delimiters with new password hash
    inputString.replace(startPos, endPos - startPos, newPassword);
    return 0;
}

int main()
{
    bool shadowOpen = false;
    // open shadow file
    std::ifstream infile("/etc/shadow", std::ifstream::in);

    if(!infile.good())
    {
        syslog(LOG_WARNING, "could not open /etc/shadow for reading");
    }
    else
    {
        syslog(LOG_WARNING, "/etc/shadow opened for reading");
        shadowOpen = true;
    }

    std::string content {std::istreambuf_iterator<char>{infile}, std::istreambuf_iterator<char>{}};
    infile.close();

    // change read-only user password
    if(shadowOpen)
    {
        int status = changeUserPassword(content, "readings", "$6$WBZqLnhfmAdun.UA$4DUwmi0VStdICkHIiUCleyhuekC863oS4CnWfXSqKc.91rsuPSqXDA3BRsMarJszx3loXPCdyDFUSsBe0pnle1");
        if(status != 0)
        {
            syslog(LOG_WARNING, "could not change 'readings' password");
        }

        std::ofstream outfile("/etc/shadow");
        if(outfile.good())
        {
            outfile << content;
            outfile.close();
        }
        else
        {
            syslog(LOG_WARNING, "could not open /etc/shadow for writing");
        }
    }

    // create json variables
    nlohmann::basic_json<>::iterator hostnameIter;
    nlohmann::basic_json<>::iterator rootPasswordIter;
    json jsonObject;

    // create file stream and set exception mask so open throws if failbit is set
    std::ifstream configFile("/mnt/data/config/host_config.json", std::fstream::in);
    if(!configFile.good())
    {
        syslog(LOG_WARNING, "Exception opening/reading file %s", hostConfigPath);
        goto endjason;
    }

    // try to parse json object
    try
    {
        jsonObject = json::parse(configFile);
    }
    catch(std::exception& e)
    {
        syslog(LOG_WARNING, "Exception parsing json file, message: %s", e.what());
        goto endjason;
    }

    // find 'hostname' entry
    hostnameIter  = jsonObject.find("hostname");
    if (hostnameIter != jsonObject.end())
    {
        std::string hostname = *hostnameIter;
        int status = sethostname(hostname.c_str(), hostname.size());
        if (status != 0)
        {
            syslog(LOG_WARNING, "Error setting host name: %s, status: %d", hostname.c_str(), status);
        }
        else
        {
            syslog(LOG_WARNING, "Host name set: %s", hostname.c_str());
        }
    }

    // find 'root_pass' entry
    rootPasswordIter  = jsonObject.find("root_pass");
    if (rootPasswordIter != jsonObject.end())
    {
        std::string rootPassword = *rootPasswordIter;

        int status = changeUserPassword(content, "root", rootPassword);
        if(status != 0)
        {
            syslog(LOG_WARNING, "could not change root password");
        }

        std::ofstream outfile("/etc/shadow");
        if(outfile.good())
        {
            outfile << content;
            outfile.close();
        }
        else
        {
            syslog(LOG_WARNING, "could not open /etc/shadow for writing");
        }
    }

endjason:
    std::error_code ec;
    std::experimental::filesystem::create_directory("/home/root/.ssh/", ec);
    if(ec)
    {
        syslog(LOG_WARNING, "could not create a directory: %s, error code: %d", "/home/root/.ssh/", ec.value());
    }
    else
    {
        syslog(LOG_WARNING, "directory successfully created: %s", "/home/root/.ssh/");
    }

    std::experimental::filesystem::copy_file("/mnt/data/config/authorized_keys", "/home/root/.ssh/authorized_keys", std::experimental::filesystem::copy_options::overwrite_existing, ec);
    if(ec)
    {
        syslog(LOG_WARNING, "file could not be copied: %s, error code: %d", "/home/root/.ssh/authorized_keys", ec.value());
    }
    else
    {
        syslog(LOG_WARNING, "file successfully copied: %s", "/home/root/.ssh/authorized_keys");
    }

    return 0;
}