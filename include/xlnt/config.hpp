// Copyright (c) 2015 Thomas Fussell
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, WRISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE
//
// @license: http://www.opensource.org/licenses/mit-license.php
// @author: see AUTHORS file
#pragma once

// Change these values for programs using this library.

namespace xlnt {

/// <summary>
/// Enumeration of possible limit styles.
/// Excel places limitations on the number of rows and columns,
/// but we may wish to change those limits in some cases. Values
/// other than excel might prevent the file from being opened in Excel.
/// </summary>
enum class limit_style
{
    openpyxl, /// limit style using in openpyxl
    excel, /// limits according to Excel
    maximum /// limits based on system
};

/// <summary>
/// The style of limits to use for reading and writing XLSX files.
/// See limit_style for more information.
/// </summary>
const limit_style LimitStyle = limit_style::openpyxl;

}
