/*
Copyright (C) 2007 Remon Sijrier

Copyright (C) 2000-2007 Paul Davis 

This file is part of Traverso

Traverso is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <fpu.h>

FPU::FPU ()
{
        unsigned long cpuflags = 0;

        _flags = Flags (0);

#ifndef ARCH_X86
        return;
#endif

#ifndef USE_X86_64_ASM
        asm volatile (
                "mov $1, %%eax\n"
                "pushl %%ebx\n"
                "cpuid\n"
                "movl %%edx, %0\n"
                "popl %%ebx\n"
                : "=r" (cpuflags)
                :
                : "%eax", "%ecx", "%edx", "memory"
                );

#else

        asm volatile (
                "pushq %%rbx\n"
                "movq $1, %%rax\n"
                "cpuid\n"
                "movq %%rdx, %0\n"
                "popq %%rbx\n"
                : "=r" (cpuflags)
                :
                : "%rax", "%rcx", "%rdx", "memory"
                );

#endif /* USE_X86_64_ASM */

        if (cpuflags & (1<<25)) {
                _flags = Flags (_flags | (HasSSE|HasFlushToZero));
        }

        if (cpuflags & (1<<26)) {
                _flags = Flags (_flags | HasSSE2);
        }

        if (cpuflags & (1 << 24)) {

                char* fxbuf = 0;
                char** temp = &fxbuf;

#ifdef NO_POSIX_MEMALIGN
                if ((fxbuf = (char *) malloc(512)) == 0)
#else
                if (posix_memalign ((void**)temp, 16, 512))
#endif
                {
                        printf("FPU() ERROR: cannot allocate 16 byte aligned buffer for h/w feature detection");
                } else {

                        asm volatile (
                                "fxsave (%0)"
                                :
                                : "r" (fxbuf)
                                : "memory"
                                );

                        uint32_t mxcsr_mask = *((uint32_t*) &fxbuf[28]);

                        /* if the mask is zero, set its default value (from intel specs) */

                        if (mxcsr_mask == 0) {
                                mxcsr_mask = 0xffbf;
                        }

                        if (mxcsr_mask & (1<<6)) {
                                _flags = Flags (_flags | HasDenormalsAreZero);
                        }

                        free (fxbuf);
                }
        }
}

FPU::~FPU ()
{
}
