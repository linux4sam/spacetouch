/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 * Joshua Henderson <joshua.henderson@microchip.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TOOLS_H
#define TOOLS_H

/**
 * @file tools.h
 * @brief Colection of functions to perform common tasks
 */

/**
 *  @brief
 *  Tools
 * */
class Tools
{
public:
    Tools();

    virtual ~Tools();

    // Stores CPU usage
    float  cpu_usage[4];

    /**
      * Updates the CPU Usage
      * The value is stored in the public cpu_usage array
      **/
    void updateCpuUsage();

private:

    // For CPU usage
    float total_cpu_last_time[4];
    float work_cpu_last_time[4];
};

#endif // TOOLS_H
