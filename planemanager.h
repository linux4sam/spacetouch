/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 * Joshua Henderson <joshua.henderson@microchip.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef PLANEMANAGER_H
#define PLANEMANAGER_H

#include <string>
#include <memory>
#include <vector>

struct plane_data;
struct kms_device;

/**
 * @brief The PlaneManager class
 *
 * This is a central manager for planes.  It uses libplanes to configure and manage planes.
 *
 * When using this class, you can choose to use the built in config and/or the engine provided
 * by libplanes, or chose not to use it.
 */
class PlaneManager
{
public:

    PlaneManager();

    /**
     * @brief Load a config file to configure the planes.
     * @param configfile
     * @return
     */
    virtual bool load(const std::string& configfile = "screen.config");

    /**
     * @brief step
     *
     * Perform an engine step if the engine is to be used.
     */
    virtual void step();

    /**
     * @brief Get a plane by name.
     * @param name
     * @return
     */
    virtual struct plane_data* get(const std::string& name);

    /**
     * @brief Get a plane by index.
     * @param index
     * @return
     */
    virtual struct plane_data* get(unsigned int index);

    virtual ~PlaneManager();

protected:

    /**
     * @brief The KMS device used to manage planes.
     */
    std::shared_ptr<kms_device> m_device;

    /**
     * @brief List of configured planes based on config file.
     */
    std::vector<plane_data*> m_planes;
};

#endif // PLANEMANAGER_H
