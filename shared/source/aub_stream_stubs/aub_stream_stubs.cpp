/*
 * Copyright (C) 2018-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aubstream/aubstream.h"

#include <limits>

namespace aub_stream {

extern "C" {
void injectMMIOList(MMIOList mmioList){};
void setTbxServerPort(uint16_t port){};
void setTbxServerIp(std::string server){};
void setTbxFrontdoorMode(bool frontdoor){};
void setAubStreamCaller(uint32_t caller){};
}

} // namespace aub_stream
