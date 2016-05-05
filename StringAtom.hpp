//
// Copyright (c) 2008-2014 OvermindDL1.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string>

namespace OverLib
{

namespace StringAtom
{

static constexpr uint8_t table_atomize[256] = {
};

namespace detail
{

constexpr uint32_t tightatomize_table[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 0,  0,  0,  0,  0,  0,
    0,  11, 12, 13, 14, 15, 16, 17, 18, 19, 0,  0,  20, 21, 22, 23,
    24, 0,  25, 26, 27, 28, 29, 30,  0, 31, 0,  0,  0,  0,  0,  0,
    0,  11, 12, 13, 14, 15, 16, 17, 18, 19, 0,  0,  20, 21, 22, 23,
    24, 0,  25, 26, 27, 28, 29, 30,  0, 31, 0,  0,  0,  0,  0,  0
};

constexpr char tightdeatomize_table[] = " 0123456789ABCDEFGHILMNOPRSTUVWY";

constexpr size_t tightatom_charwidth = 5;
constexpr size_t tightatom_charwidthmask32 = 31;
constexpr size_t tightatom_charlen32 = 6;

//*
constexpr uint64_t atomize_table64[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  0,  0,  0,  0,  0,  0,
    0,  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 0,  0,  0,  0,  37,
    0,  38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0,  0,  0,  0,  0
};

constexpr char deatomize_table[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
/*/
constexpr uint64_t atomize_table64[] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
    47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 0,  59, 60, 61,
     0, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 62,  0, 63,  0,  0
};

constexpr char deatomize_table[] = " !#$%&'()*+,-./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:;<=>?@[]^_{}";
//*/

constexpr size_t atom_charwidth = 6;
constexpr size_t atom_charwidthmask64 = 63;
constexpr size_t atom_charlen64 = 10;

}


// Can hold 6 characters
typedef uint32_t TightAtom32;
// Can hold 10 characters
typedef uint64_t Atom64;


constexpr
static TightAtom32 tightatomize32(const char* input, size_t offset = 0)
{
    return ((*input) == 0 || offset > detail::tightatom_charwidthmask32)
           ? 0
           : tightatomize32(input + 1, offset + detail::tightatom_charwidth) ^ (detail::tightatomize_table[(int)*input] << offset);
}

static std::string tightdeatomize32(TightAtom32 atom)
{
    std::string ret(detail::tightatom_charlen32,  ' ');
    for (size_t i = 0; i < detail::tightatom_charlen32; ++i) {
        ret[i] = detail::tightdeatomize_table[
                     (atom & (detail::tightatom_charwidthmask32 << (i * detail::tightatom_charwidth))) >> (i * detail::tightatom_charwidth)
                 ];
    }
    return ret;
}

constexpr
static Atom64 atomize64(const char* input, size_t offset = 0)
{
    return ((*input) == 0 || offset > detail::atom_charwidthmask64)
           ? 0
           : atomize64(input + 1, offset + detail::atom_charwidth) ^ (detail::atomize_table64[(int)*input] << offset);
}

// TODO:  Figure out magic syntax to get this to work on mingw builds and all other builds...  >.>
//*
constexpr Atom64 operator "" _atom64(const char* str, size_t len)
{
    return atomize64(str);
}
/*/
constexpr Atom64 operator "" _atom64(const char* str, long long unsigned int what)
{
    return atomize64(str);
}
// */

    static Atom64 atomize64(const std::string input)
{
	return atomize64(input.c_str());
}

    static std::string deatomize64(Atom64 atom)
{
    std::string ret(detail::atom_charlen64,  ' ');
    for (size_t i = 0; i < detail::atom_charlen64; ++i) {
        ret[i] = detail::deatomize_table[
                     (atom & (detail::atom_charwidthmask64 << (i * detail::atom_charwidth))) >> (i * detail::atom_charwidth)
                 ];
    }
    return ret;
}


}

}
/*
constexpr uint32_t TIGHTATOM_ATOM = OverLib::StringAtom::tightatomize32("ATOM");
constexpr uint64_t ATOM_ATOM = OverLib::StringAtom::atomize64("ATOM");

static int testing()
{
    using namespace OverLib::StringAtom;
    TightAtom32 tightatom;
    Atom64 atom;
    Urho3D::String s;
    tightatom = tightatomize32("0");
    atom = atomize64("0");
    s = tightdeatomize32(tightatom);
    s = deatomize64(atom);
    tightatom = tightatomize32("00");
    atom = atomize64("00");
    s = tightdeatomize32(tightatom);
    s = deatomize64(atom);
    tightatom = tightatomize32("BTOM");
    atom = atomize64("BTOM");
    s = tightdeatomize32(tightatom);
    s = tightdeatomize32(TIGHTATOM_ATOM);
    s = deatomize64(atom);
    s = deatomize64(ATOM_ATOM);
    tightatom = tightatomize32("Way too long");
    atom = atomize64("Another that is way way too long");
    Atom64 a = atomize64("Anoth  ");
    Atom64 b = atomize64("      e");
    Atom64 c = atomize64("Anoth e");
    s = tightdeatomize32(tightatom);
    s = deatomize64(atom);
    s = deatomize64(a);
    s = deatomize64(b);
    s = deatomize64(c);
    Atom64 d0 = "test"_atom64;
    Atom64 d1 = atomize64("test");
    assert(d0 == d1);
}

static int blah = testing();
// */
