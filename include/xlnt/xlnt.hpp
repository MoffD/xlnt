// Copyright (c) 2015 Thomas Fussell
// Copyright (c) 2010-2015 openpyxl
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

#include <string>

const std::string xlnt_version = "0.1.0";

const std::string author = "Thomas Fussell";
const std::string license = "MIT";
const std::string author_email = "thomas.fussell@gmail.com";
const std::string url = "https://github.com/tfussell/xlnt";
const std::string download_url = "https://github.com/tfussell/xlnt/archive/master.zip";

#include "cell/cell.hpp"
#include "cell/cell_reference.hpp"
#include "cell/comment.hpp"
#include "common/datetime.hpp"
#include "common/encoding.hpp"
#include "common/exceptions.hpp"
#include "common/relationship.hpp"
#include "common/string_table.hpp"
#include "common/zip_file.hpp"
#include "reader/excel_reader.hpp"
#include "workbook/document_properties.hpp"
#include "workbook/named_range.hpp"
#include "workbook/workbook.hpp"
#include "worksheet/range.hpp"
#include "worksheet/range_reference.hpp"
#include "worksheet/worksheet.hpp"
#include "writer/workbook_writer.hpp"
