/*
 * GridTools
 *
 * Copyright (c) 2020, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "../thread_pool/hpx.hpp"
#include "cpu_ifirst.hpp"

namespace gridtools {
    namespace stencil {
        using cpu_ifirst_hpx = cpu_ifirst<thread_pool::hpx>;
    } // namespace stencil
} // namespace gridtools
