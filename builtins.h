/* builtins.h
 * Explicitly map gcc builtins.
 *
 * There are some functions we need that aren't in the Arduino libc, but
 * are available as gcc builtins. Since they aren't in libc, we can't
 * include the usual headers, so instead explicitly define them to the
 * builtins here.
 */

#ifndef __BUILTINS_H
#define __BUILTINS_H

#define vsnprintf __builtin_vsnprintf

#endif
