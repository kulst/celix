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

use celix_status_t;

pub const BUNDLE_EXCEPTION: celix_status_t = 70001; //TODO move to celix_status_t_CELIX_BUNDLE_EXCEPTION

pub enum Error {
    BundleException,
    CelixStatusError(celix_status_t), // Represent unexpected C API errors
}

impl From<celix_status_t> for Error {
    fn from(status: celix_status_t) -> Self {
        match status {
            BUNDLE_EXCEPTION => Error::BundleException,
            _ => Error::CelixStatusError(status),
        }
    }
}

impl Into<celix_status_t> for Error {
    fn into(self) -> celix_status_t {
        match self {
            Error::BundleException => BUNDLE_EXCEPTION,
            Error::CelixStatusError(status) => status,
        }
    }
}
