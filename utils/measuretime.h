//
//  measuretime.h
//  common
//
//  Created by Albrecht Eckardt on 2018-07-26.
//  Copyright © 2018 Albrecht Eckardt. All rights reserved.
//

#ifndef measuretime_h
#define measuretime_h

#include <stdio.h>
#include <iostream>
#include <sys/time.h>

#define InitMeasurement() \
    struct timeval start, end; \
    long mtime, seconds, useconds;
    
#define StartMeasurement() \
    gettimeofday(&start, NULL); \
    mtime = 0;

#define StopMeasurement() \
    gettimeofday(&end, NULL); \
    seconds  = end.tv_sec  - start.tv_sec; \
    useconds = end.tv_usec - start.tv_usec; \
    mtime = static_cast<long>((static_cast<double>(seconds) * 1000 + static_cast<double>(useconds) / 1000.0) + 0.5);
    
#define StopEvaluateMeasurement(cycles, it_name) \
    StopMeasurement(); \
    if (mtime / cycles < 2) \
    printf("%s: %4.2f ms\n", it_name, static_cast<double>(mtime) / static_cast<double>(cycles)); \
    else \
    printf("%s: %d ms\n", it_name, static_cast<int>(mtime) / static_cast<int>(cycles));
    
#define Iterate(fnc_call, cycles, it_name) \
    StartMeasurement(); \
    { \
        int it_n; \
        for (it_n = 0; it_n < cycles; it_n++) \
        { \
            fnc_call; \
        } \
    } \
    StopEvaluateMeasurement(cycles, it_name);
    
#endif /* measuretime_h */
