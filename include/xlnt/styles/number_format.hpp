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
#include <unordered_map>
#include <utility>

namespace xlnt {

class number_format
{
public:
    enum class format
    {
        general,
        text,
        number,
        number_00,
        number_comma_separated1,
        number_comma_separated2,
        percentage,
        percentage_00,
        date_yyyymmdd2,
        date_yyyymmdd,
        date_ddmmyyyy,
        date_dmyslash,
        date_dmyminus,
        date_dmminus,
        date_myminus,
        date_xlsx14,
        date_xlsx15,
        date_xlsx16,
        date_xlsx17,
        date_xlsx22,
        date_datetime,
        date_time1,
        date_time2,
        date_time3,
        date_time4,
        date_time5,
        date_time6,
        date_time7,
        date_time8,
        date_timedelta,
        date_yyyymmddslash,
        currency_usd_simple,
        currency_usd,
        currency_eur_simple,
        unknown
    };

    struct format_hash
    {
        std::size_t operator()(format f) const
        {
	    return std::hash<int>()((int)f);
	}
    };
    
    static const std::unordered_map<format, std::string, format_hash> &format_strings();    
    static const std::unordered_map<int, std::string> &builtin_formats();
    static const std::unordered_map<std::string, int> &reversed_builtin_formats();    

    static std::string builtin_format_code(int index);
    static format lookup_format(int code);
    
    static bool is_builtin(const std::string &format);
    
    static const number_format &default_number_format()
    {
        static number_format default_;
        return default_;
    }
    
    number_format();
    number_format(format code);
    number_format(const std::string &code);
    
    format get_format_code() const;
    
    void set_format_code(format format_code);
    
    void set_format_string(const std::string &format_code);
    std::string get_format_string() const;
    
    int get_format_index() const { return format_index_; }
    
    std::size_t hash() const;
    
    bool operator==(const number_format &other) const
    {
        return hash() == other.hash();
    }

private:
    format format_code_;
    int format_index_;
    std::string format_string_;
};

} // namespace xlnt
