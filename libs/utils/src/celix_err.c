/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "celix_err.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

pthread_key_t celix_err_tssKey; //TODO replace with celix_tss_key and celix_tss_* functions

typedef struct celix_err {
    char buffer[CELIX_ERR_BUFFER_SIZE];
    size_t pos;
} celix_err_t;

static void celix_err_destroyTssErr(void* data) {
    celix_err_t* err = data;
    if (err != NULL) {
        free(err);
    }
}

static celix_err_t* celix_err_getTssErr() {
    celix_err_t* err = pthread_getspecific(celix_err_tssKey);
    if (err) {
        return err;
    }

    err = malloc(sizeof(*err));
    if (err) {
        err->pos = 0; //no entry
        int rc = pthread_setspecific(celix_err_tssKey, err);
        if (rc != 0) {
            fprintf(stderr, "Failed to set thread specific storage for celix_err: %s\n", strerror(rc));
            free(err);
            err = NULL;
        }
    } else {
        fprintf(stderr, "Failed to allocate memory for celix_err\n");
    }
    return err;
}

__attribute__((constructor)) void celix_err_initThreadSpecificStorageKey() {
    int rc = pthread_key_create(&celix_err_tssKey, celix_err_destroyTssErr);
    if (rc != 0) {
        fprintf(stderr,"Failed to create thread specific storage key for celix_err\n");
    }
}

__attribute__((destructor)) void celix_err_deinitThreadSpecificStorageKey() {
    int rc = pthread_key_delete(celix_err_tssKey);
    if (rc != 0) {
        fprintf(stderr,"Failed to delete thread specific storage key for celix_err\n");
    }
}

const char* celix_err_popLastError() {
    const char* result = NULL;
    celix_err_t* err = celix_err_getTssErr();
    if (err && err->pos > 0) {
        //move back to start last error message
        err->pos -= 1; //move before \0 in last error message
        while (err->pos > 0 && err->buffer[err->pos-1] != '\0') {
            err->pos -= 1;
        }
        result = err->buffer + err->pos;
    }
    return result;
}

int celix_err_getErrorCount() {
    int result = 0;
    celix_err_t* err = celix_err_getTssErr();
    for (int i = 0; err && i < err->pos; ++i) {
        if (err->buffer[i] == '\0') {
            result += 1;
        }
    }
    return result;
}

void celix_err_resetErrors() {
    void* data = pthread_getspecific(celix_err_tssKey);
    if (data != NULL) {
        celix_err_t* err = data;
        err->pos = 0; //no entry
    }
}

void celix_err_push(const char* msg) {
    celix_err_t* err = celix_err_getTssErr();
    if (err) {
        size_t len = strnlen(msg, CELIX_ERR_BUFFER_SIZE);
        if (err->pos + len + 1 <= CELIX_ERR_BUFFER_SIZE) {
            strcpy(err->buffer + err->pos, msg);
            err->pos += len + 1;
        } else {
            fprintf(stderr, "Failed to add error message '%s' to celix_err\n", msg);
        }
    }
}

void celix_err_pushf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    celix_err_t* err = celix_err_getTssErr();
    if (err) {
        size_t len = vsnprintf(err->buffer + err->pos, CELIX_ERR_BUFFER_SIZE - err->pos, format, args);
        if (err->pos + len + 1 <= CELIX_ERR_BUFFER_SIZE) {
            err->pos += len;
            err->buffer[err->pos++] = '\0';
        } else {
            fprintf(stderr, "Failed to add error message '");
            vfprintf(stderr, format, args);
            fprintf(stderr, "' to celix_err\n");

        }
    }
    va_end(args);
}
