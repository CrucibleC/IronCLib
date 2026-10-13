/*
 * Copyright 2026 CrucibleC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IC_VERSION_H
#define IC_VERSION_H

/*
===============================================================================
IC Version

The IronCLib version, following semantic versioning (MAJOR.MINOR.PATCH):
- MAJOR: incompatible changes
- MINOR: new, backwards-compatible features
- PATCH: backwards-compatible fixes

This header is the single source of truth for the version. A release is made by
pushing a tag vMAJOR.MINOR.PATCH, and the release workflow fails if the tag does
not match IC_VERSION_STRING below, or the version in the README title.

Usage:
    #if IC_VERSION_NUMBER >= IC_MAKE_VERSION_NUMBER(1, 0, 3)
        // code relying on 1.0.3 features
    #endif
===============================================================================
*/

#define IC_VERSION_MAJOR 1
#define IC_VERSION_MINOR 0
#define IC_VERSION_PATCH 3
#define IC_VERSION_STRING "1.0.3"

// Comparable version number, e.g. 1.1.0 -> 10100
#define IC_MAKE_VERSION_NUMBER(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))
#define IC_VERSION_NUMBER IC_MAKE_VERSION_NUMBER(IC_VERSION_MAJOR, IC_VERSION_MINOR, IC_VERSION_PATCH)

#endif // IC_VERSION_H
