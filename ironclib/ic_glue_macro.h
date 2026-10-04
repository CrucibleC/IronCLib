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

#ifndef IC_GLUE_MACRO_H
#define IC_GLUE_MACRO_H

// This header is used internally within the library.

#define IC_INNER_GLUE_IMPL(a, b) a##b
#define IC_GLUE(a, b) IC_INNER_GLUE_IMPL(a, b)
#define IC_GLUE3(a, b, c) IC_GLUE(IC_GLUE(a, b), c)
#define IC_GLUE4(a, b, c, d) IC_GLUE(IC_GLUE3(a, b, c), d)
#define IC_GLUE5(a, b, c, d, e) IC_GLUE(IC_GLUE4(a, b, c, d), e)
#define IC_GLUE6(a, b, c, d, e, f) IC_GLUE(IC_GLUE5(a, b, c, d, e), f)
#define IC_GLUE7(a, b, c, d, e, f, g) IC_GLUE(IC_GLUE6(a, b, c, d, e, f), g)
#define IC_GLUE8(a, b, c, d, e, f, g, h) IC_GLUE(IC_GLUE7(a, b, c, d, e, f, g), h)
#define IC_GLUE9(a, b, c, d, e, f, g, h, i) IC_GLUE(IC_GLUE8(a, b, c, d, e, f, g, h), i)


#endif // IC_GLUE_MACRO_H