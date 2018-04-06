/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 * Joshua Henderson <joshua.henderson@microchip.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "planemanager.h"
#include <planes/plane.h>
#include <planes/engine.h>
#include <planes/kms.h>
#include <QApplication>
#include <QDebug>
#include <qpa/qplatformnativeinterface.h>

/**
 * @brief This requires a custom version of Qt to work with a patch for getting the DRI
 * file descriptor used internally by the linuxfb backend configured in DRM mode.
 */
static int get_dri_fd()
{
    static int dri_fd = -1;
    if (dri_fd == -1)
    {
        void *p = reinterpret_cast<QApplication*>(QApplication::instance())->
                platformNativeInterface()->nativeResourceForIntegration("dri_fd");
        if (p)
            dri_fd = (int)(qintptr)p;
    }
    return dri_fd;
}

PlaneManager::PlaneManager()
{
}

bool PlaneManager::load(const std::string& configfile)
{
    int fd = get_dri_fd();
    if (fd < 0)
    {
        qDebug() << "No dri fd available from platform";
        return false;
    }
    else
    {
        qDebug() << "dri fd = " << fd;
    }

    m_device.reset(kms_device_open(fd));
    if (!m_device)
        return false;

    m_planes.resize(m_device->num_planes, 0);

    return !(engine_load_config(configfile.c_str(), m_device.get(), m_planes.data(), m_planes.size(), 0));
}

void PlaneManager::step()
{
    engine_run_once(m_device.get(), m_planes.data(), m_planes.size(), 0);
}

struct plane_data* PlaneManager::get(const std::string& name)
{
    for (auto i: m_planes)
        if (i)
            if (std::string(i->name) == name)
                return i;

    return 0;
}

struct plane_data* PlaneManager::get(unsigned int index)
{
    if (index < m_planes.size())
        return m_planes[index];

    return 0;
}

PlaneManager::~PlaneManager()
{
    for (auto i: m_planes)
        if (i)
            free(i);

    if (m_device)
        kms_device_close(m_device.get());
}
