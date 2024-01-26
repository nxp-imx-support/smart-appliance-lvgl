/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdio>
#include <iostream>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <unistd.h>
#include "log_parse.h"

/*
type 1 to read status of onoff device
type 2 to read value  of network status
*/
int parse_log(std::string log, int type, bool* data )
{
    int index;

    switch(type) {
        case 1:
            index = log.find("OnOff:");
            if(index != std::string::npos) {
                if(log.find("FALSE") != std::string::npos) {
                    *data = false;
                } else {
                    *data = true;
                }
                return 1;
            }
            break;

        case 2:
            index = log.find("Bcast:");
            if(index != std::string::npos) {
                *data = true;
                return 1;
            }
            break;

        default:
            *data = false;
            break;
    }

    return 0;
}

int parse_log(std::string log, char* data )
{
    int index;
    // Temperature sensor
    index = log.find("MeasuredValue:");
    if(index != std::string::npos) {
        std::memcpy((char*)data, log.c_str() + index + 15, 2);
        data[2] = '.';
        data[3] = log[index + 18];
        std::memcpy((char*)data + 4, "°C", sizeof("°C"));
        return 1;
    }

    return 0;
}

int send_cmd(const char* cmd)
{
    int ret;
    ret = system(cmd);
    return ret;
}

int read_bool(const char* cmd, int type, bool* back_data)
{
    std::array<char, 128> buffer;
    int ret = 0;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            ret = parse_log(buffer.data(), type, back_data);
            if(ret > 0)
                return ret;
        }
    }
    return 0;
}

int read_string(const char* cmd, char* back_data)
{
    std::array<char, 128> buffer;
    int ret = 0;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            ret = parse_log(buffer.data(), back_data);
            if(ret > 0)
                return ret;
        }
    }
    return 0;
}


int interactive_start(const char* cmd, FILE** fp)
{
    *fp=popen(cmd,"w");
    if(*fp == NULL) {
        perror("popen error\n");
        return -1;
    }

    return 0;
}

int interactive_end(FILE *fp)
{
    pclose(fp);
    return 0;
}

int input_cmd(const char* cmd, FILE* fp)
{
    fprintf(fp, "%s \n",cmd);
    fflush(fp);

    return 0;
}

int output_bool(const char* cmd, FILE* fp, bool*  back_data)
{
    return 0;
}

int output_string(const char* cmd, FILE* fp, char* back_data)
{
    return 0;
}
