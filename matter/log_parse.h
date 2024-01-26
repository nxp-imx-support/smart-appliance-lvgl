/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LOG_PARSE_H_
#define LOG_PARSE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

int send_cmd(const char* cmd);
int read_bool(const char* cmd, int type, bool* back_data);
int read_string(const char* cmd, char* back_data);

int interactive_start(const char* cmd, FILE** fp);
int interactive_end(FILE *fp);
int input_cmd(const char* cmd, FILE* fp);
int output_bool(const char* cmd, FILE* fp, bool* back_data);
int output_string(const char* cmd, FILE* fp, char* back_data);

#ifdef __cplusplus
}
#endif
#endif /* LOG_PARSE_H_ */
