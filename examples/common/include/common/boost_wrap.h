//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/version.hpp>
#include <boost/asio.hpp>

namespace cc_demo1
{

namespace common
{

namespace boost_wrap
{

#define DEMO1_BOOST_MAJOR (BOOST_VERSION / 100000)
#define DEMO1_BOOST_MINOR ((BOOST_VERSION / 100) % 1000)

#if (DEMO1_BOOST_MAJOR != 1)
#warning "Unexpected boost major version"
#endif

#if (DEMO1_BOOST_MINOR < 66)

using io = boost::asio::io_service;

template <typename TFunc>
void post(io& i, TFunc&& func)
{
    i.post(std::forward<TFunc>(func));
}

#else // #if (DEMO1_BOOST_MINOR < 66)

using io = boost::asio::io_context;

template <typename TFunc>
void post(io& i, TFunc&& func)
{
    boost::asio::post(i, std::forward<TFunc>(func));
}

#endif // #if (DEMO1_BOOST_MINOR < 66)

} // namespace boost_wrap


} // namespace common


} // namespace cc_demo1
