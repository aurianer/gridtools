/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <gtest/gtest.h>

#ifdef GT_STENCIL_CPU_IFIRST_HPX
    #include <hpx/hpx_start.hpp>
    #include <hpx/include/run_as.hpp>
#endif

#ifdef GT_STENCIL_CPU_IFIRST_HPX
    class test_base : public ::testing::Test {
        protected:
            static void SetUpTestCase() {
                // FIXME: how to manage to pass the argc argv
                hpx::start(nullptr, hpx::detail::dummy_argc,
                        hpx::detail::dummy_argv);
            }
            static void TearDownTestCase() {
                hpx::apply([]() { hpx::finalize(); });
                hpx::stop();
            }
    };
#else
    using test_base = ::testing::Test;
#endif
