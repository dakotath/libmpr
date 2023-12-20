#pragma once
#ifndef VERSIONS_H
#define VERSIONS_H

#define GCWII_VERSION "MPRender V1.0.1 for GCWII"
#define WIN_VERSION "MPRender V1.0.1 for Windows"
#define LINUX_VERSION "MPRender V1.0.1 for Linux"
#define MAC_VERSION "MPRender V1.0.1 for Mac"

#ifdef CC_BUILD_WIN
#define CURRENT_VERSION WIN_VERSION
#elif defined(CC_BUILD_LINUX)
#define CURRENT_VERSION LINUX_VERSION
#elif defined(CC_BUILD_MAC)
#define CURRENT_VERSION MAC_VERSION
#elif defined(CC_BUILD_GCWII)
#define CURRENT_VERSION GCWII_VERSION
#else
#define CURRENT_VERSION "MPRender V1.0.1 for Unknown Platform"
#endif

#endif