// ISC License
//
// Copyright (c) 2017 by Alexej Harm
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS AL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

// Portions Copyright (c) 1996-1999 by Internet Software Consortium.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
// ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
// CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
// PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
// ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
// SOFTWARE.

// Portions Copyright (c) 1995 by International Business Machines, Inc.
//
// International Business Machines, Inc. (hereinafter called IBM) grants
// permission under its copyrights to use, copy, modify, and distribute this
// Software with or without fee, provided that the above copyright notice and
// all paragraphs of this notice appear in all copies, and that the name of IBM
// not be used in connection with the marketing of any product incorporating
// the Software or modifications thereof, without specific, written prior
// permission.
//
// To the extent it has a right to do so, IBM grants an immunity from suit
// under its patents, if any, for the use, sale or manufacture of products to
// the extent that such products are used for performing Domain Name System
// dynamic updates in TCP/IP networks by means of the Software.  No immunity is
// granted for any product per se or for any other function of any product.
//
// THE SOFTWARE IS PROVIDED "AS IS", AND IBM DISCLAIMS ALL WARRANTIES,
// INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  IN NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL,
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER ARISING
// OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE, EVEN
// IF IBM IS APPRISED OF THE POSSIBILITY OF SUCH DAMAGES.

#pragma once
#include <algorithm>
#include <string>
#include <string_view>
#include <cctype>
#include <cstdint>

namespace ice {
namespace base {

constexpr const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr const char padding = '=';

inline std::string encode(std::string_view data)
{
  auto src = data.data();
  auto size = data.size();
  std::string dst;
  dst.resize(4 * (size / 3) + (size % 3 != 0 ? 4 : 0));
  std::size_t pos = 0;
  std::uint8_t o[3];
  while (2 < size) {
    o[0] = static_cast<std::uint8_t>(*src++);
    o[1] = static_cast<std::uint8_t>(*src++);
    o[2] = static_cast<std::uint8_t>(*src++);
    size -= 3;
    dst[pos++] = table[o[0] >> 2];
    dst[pos++] = table[((o[0] & 0x03) << 4) + (o[1] >> 4)];
    dst[pos++] = table[((o[1] & 0x0f) << 2) + (o[2] >> 6)];
    dst[pos++] = table[o[2] & 0x3f];
  }
  if (size != 0) {
    o[0] = size > 0 ? static_cast<std::uint8_t>(*src++) : 0;
    o[1] = size > 1 ? static_cast<std::uint8_t>(*src++) : 0;
    o[2] = size > 2 ? static_cast<std::uint8_t>(*src++) : 0;
    dst[pos++] = table[o[0] >> 2];
    dst[pos++] = table[((o[0] & 0x03) << 4) + (o[1] >> 4)];
    dst[pos++] = size == 1 ? padding : table[((o[1] & 0x0f) << 2) + (o[2] >> 6)];
    dst[pos++] = padding;
  }
  return dst;
}

inline std::string encode(std::basic_string_view<unsigned char> data)
{
  return encode(std::string_view(reinterpret_cast<const char*>(data.data()), data.size()));
}

inline std::string decode(std::string_view data)
{
  constexpr auto begin = std::begin(table);
  constexpr auto end = std::end(table);
  std::string dst;
  dst.reserve(data.size() / 4 * 3);
  auto state = 0;
  for (const auto c : data) {
    if (std::isspace(c)) {
      continue;  // skip spaces
    }
    if (c == padding) {
      if (state > 1) {
        dst.resize(dst.size() - 1);
      }
      break;  // ignore padding
    }
    const auto it = std::find(begin, end, c);
    if (it == end) {
      return {};
    }
    const auto value = static_cast<std::uint8_t>(it - begin);
    switch (state) {
    case 0:
      dst.push_back(value << 2);
      state = 1;
      break;
    case 1:
      dst.back() |= value >> 4;
      dst.push_back((value & 0x0f) << 4);
      state = 2;
      break;
    case 2:
      dst.back() |= value >> 2;
      dst.push_back((value & 0x03) << 6);
      state = 3;
      break;
    case 3:
      dst.back() |= value;
      state = 0;
      break;
    }
  }
  return dst;
}

inline std::string decode(std::basic_string_view<unsigned char> data)
{
  return decode(std::string_view(reinterpret_cast<const char*>(data.data()), data.size()));
}

}  // namespace base
}  // namespace ice
