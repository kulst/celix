/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
/*
 * add_command.c
 *
 *  \date       Oct 13, 2011
 *  \author    	<a href="mailto:celix-dev@incubator.apache.org">Apache Celix Project Team</a>
 *  \copyright	Apache License, Version 2.0
 */

#include <stdlib.h>
#include <ctype.h>

#include <apr_strings.h>

#include "command_private.h"
#include "array_list.h"
#include "bundle_context.h"
#include "add_command.h"
#include "example_service.h"


void addCommand_execute(COMMAND command, char * line, void (*out)(char *), void (*err)(char *));
celix_status_t addCommand_isNumeric(COMMAND command, char *number, bool *ret);

COMMAND addCommand_create(BUNDLE_CONTEXT context) {
	apr_pool_t *pool;
	bundleContext_getMemoryPool(context, &pool);

    COMMAND command = (COMMAND) apr_palloc(pool, sizeof(*command));
    if (command) {
		command->bundleContext = context;
		command->name = "add";
		command->shortDescription = "add the given doubles";
		command->usage = "add <double> <double>";
		command->executeCommand = addCommand_execute;
    }
    return command;
}

void addCommand_destroy(COMMAND command) {
}

void addCommand_execute(COMMAND command, char *line, void (*out)(char *), void (*err)(char *)) {
	celix_status_t status = CELIX_SUCCESS;
    SERVICE_REFERENCE exampleService = NULL;

    status = bundleContext_getServiceReference(command->bundleContext, (char *) EXAMPLE_SERVICE, &exampleService);
    if (status == CELIX_SUCCESS) {
    	char *token;
		char *commandStr = apr_strtok(line, " ", &token);
		char *aStr = apr_strtok(NULL, " ", &token);
		bool numeric;
		addCommand_isNumeric(command, aStr, &numeric);
		if (aStr != NULL && numeric) {
			char *bStr = apr_strtok(NULL, " ", &token);
			addCommand_isNumeric(command, bStr, &numeric);
			if (bStr != NULL && numeric) {
				example_service_t example = NULL;
				status = bundleContext_getService(command->bundleContext, exampleService, (void *) &example);
				if (status == CELIX_SUCCESS) {
					double a = atof(aStr);
					double b = atof(bStr);
					double result = 0;
					status = example->add(example->example, a, b, &result);
					if (status == CELIX_SUCCESS) {
						char line[256];
						sprintf(line, "CALCULATOR_SHELL: Add: %f + %f = %f\n", a, b, result);
						out(line);
					} else {
						out("ADD: Unexpected exception in Calc service\n");
					}
				} else {
					out("No calc service available\n");
				}
			} else {
				out("ADD: Requires 2 numerical parameter\n");
			}
		} else {
			out("ADD: Requires 2 numerical parameter\n");
			status = CELIX_ILLEGAL_ARGUMENT;
		}

        double a;
        double b;
    } else {
        out("No calc service available\n");
    }

    //return status;
}

celix_status_t addCommand_isNumeric(COMMAND command, char *number, bool *ret) {
	celix_status_t status = CELIX_SUCCESS;
	*ret = true;
	while(*number) {
		if(!isdigit(*number) && *number != '.') {
			*ret = false;
			break;
		}
		number++;
	}
	return status;
}
