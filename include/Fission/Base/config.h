#pragma once

#define _FISSION_BASE_BEGIN namespace Fission{namespace base{
#define _FISSION_BASE_END }}

#define _FISSION_BASE_PUBLIC_BEGIN namespace Fission{
#define _FISSION_BASE_PUBLIC_END }

#define _FISSION_BASE_ALIASES(N) \
using N##f = N<float>;            \
using N##d = N<double>;            \
using N##l = N<long long>;          \
using N##i = N<int>

_FISSION_BASE_BEGIN

using size_t =:: size_t; // lol

_FISSION_BASE_END
