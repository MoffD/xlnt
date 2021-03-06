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
#include <vector>

#include <xlnt/writer/style_writer.hpp>

namespace xlnt {

class workbook;
class zip_file;

class excel_writer
{
public:
    excel_writer(workbook &wb);
    
    void save(const std::string &filename, bool as_template);
    void write_data(zip_file &archive, bool as_template);
    void write_string_table(zip_file &archive);
    void write_images(zip_file &archive);
    void write_charts(zip_file &archive);
    void write_chartsheets(zip_file &archive);
    void write_worksheets(zip_file &archive);
    void write_external_links(zip_file &archive);
    
private:
    workbook wb_;
    style_writer style_writer_;
    std::vector<std::string> shared_strings_;
};

std::string write_shared_strings(const std::vector<std::string> &string_table);
std::string write_properties_core(const document_properties &prop);
std::string write_worksheet_rels(worksheet ws);
std::string write_theme();
std::string write_properties_app(const workbook &wb);
std::string write_root_rels(const workbook &wb);
std::string write_workbook(const workbook &wb);
std::string write_workbook_rels(const workbook &wb);
std::string write_defined_names(const xlnt::workbook &wb);
    
bool save_workbook(workbook &wb, const std::string &filename, bool as_template = false);
std::vector<std::uint8_t> save_virtual_workbook(xlnt::workbook &wb, bool as_template = false);

} // namespace xlnt
