#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <xlnt/common/exceptions.hpp>
#include <xlnt/common/relationship.hpp>
#include <xlnt/common/zip_file.hpp>
#include <xlnt/drawing/drawing.hpp>
#include <xlnt/reader/shared_strings_reader.hpp>
#include <xlnt/reader/workbook_reader.hpp>
#include <xlnt/reader/worksheet_reader.hpp>
#include <xlnt/styles/alignment.hpp>
#include <xlnt/styles/border.hpp>
#include <xlnt/styles/fill.hpp>
#include <xlnt/styles/font.hpp>
#include <xlnt/styles/number_format.hpp>
#include <xlnt/styles/protection.hpp>
#include <xlnt/workbook/document_properties.hpp>
#include <xlnt/workbook/named_range.hpp>
#include <xlnt/workbook/workbook.hpp>
#include <xlnt/worksheet/range.hpp>
#include <xlnt/worksheet/worksheet.hpp>
#include <xlnt/writer/style_writer.hpp>
#include <xlnt/writer/manifest_writer.hpp>
#include <xlnt/writer/worksheet_writer.hpp>
#include <xlnt/writer/workbook_writer.hpp>

#include "detail/cell_impl.hpp"
#include "detail/include_pugixml.hpp"
#include "detail/workbook_impl.hpp"
#include "detail/worksheet_impl.hpp"

namespace {
    
static std::string create_temporary_filename()
{
#ifdef _WIN32
    std::array<TCHAR, MAX_PATH> buffer;
    DWORD result = GetTempPath(static_cast<DWORD>(buffer.size()), buffer.data());
    if(result > MAX_PATH)
    {
        throw std::runtime_error("buffer is too small");
    }
    if(result == 0)
    {
        throw std::runtime_error("GetTempPath failed");
    }
    std::string directory(buffer.begin(), buffer.begin() + result);
    return directory + "xlnt.xlsx";
#else
    return "/tmp/xlsx.xlnt";
#endif
}

template <class T>
void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
    
} // namespace

namespace xlnt {
namespace detail {

workbook_impl::workbook_impl() : active_sheet_index_(0), guess_types_(false), data_only_(false)
{
}

} // namespace detail
    
workbook::workbook() : d_(new detail::workbook_impl())
{
    create_sheet("Sheet");
    create_relationship("rId2", "sharedStrings.xml", relationship::type::shared_strings);
    create_relationship("rId3", "styles.xml", relationship::type::styles);
    create_relationship("rId4", "theme/theme1.xml", relationship::type::theme);
}

workbook::iterator::iterator(workbook &wb, std::size_t index) : wb_(wb), index_(index)
{
    
}

workbook::iterator::iterator(const iterator &rhs) : wb_(rhs.wb_), index_(rhs.index_)
{

}

worksheet workbook::iterator::operator*()
{
    return wb_[index_];
}
    
workbook::iterator &workbook::iterator::operator++()
{
    index_++;
    return *this;
}
    
workbook::iterator workbook::iterator::operator++(int)
{
    iterator old(wb_, index_);
    ++*this;
    return old;
}
    
bool workbook::iterator::operator==(const iterator &comparand) const
{
    return index_ == comparand.index_ && wb_ == comparand.wb_;
}
    
workbook::const_iterator::const_iterator(const workbook &wb, std::size_t index) : wb_(wb), index_(index)
{
    
}

workbook::const_iterator::const_iterator(const const_iterator &rhs) : wb_(rhs.wb_), index_(rhs.index_)
{

}

const worksheet workbook::const_iterator::operator*()
{
    return wb_.get_sheet_by_index(index_);
}

workbook::const_iterator &workbook::const_iterator::operator++()
{
    index_++;
    return *this;
}

workbook::const_iterator workbook::const_iterator::operator++(int)
{
    const_iterator old(wb_, index_);
    ++*this;
    return old;
}
    
bool workbook::const_iterator::operator==(const const_iterator &comparand) const
{
    return index_ == comparand.index_ && wb_ == comparand.wb_;
}
    
worksheet workbook::get_sheet_by_name(const std::string &name)
{
    for(auto &impl : d_->worksheets_)
    {
        if(impl.title_ == name)
        {
            return worksheet(&impl);
        }
    }

    return worksheet();
}

worksheet workbook::get_sheet_by_index(std::size_t index)
{
    return worksheet(&d_->worksheets_[index]);
}
    
const worksheet workbook::get_sheet_by_index(std::size_t index) const
{
    return worksheet(&d_->worksheets_.at(index));
}

worksheet workbook::get_active_sheet()
{
    return worksheet(&d_->worksheets_[d_->active_sheet_index_]);
}

bool workbook::has_named_range(const std::string &name) const
{
    for(auto worksheet : *this)
    {
        if(worksheet.has_named_range(name))
        {
            return true;
        }
    }
    return false;
}

worksheet workbook::create_sheet()
{   
    std::string title = "Sheet1";
    int index = 1;

    while(get_sheet_by_name(title) != nullptr)
    {
        title = "Sheet" + std::to_string(++index);
    }

    d_->worksheets_.push_back(detail::worksheet_impl(this, title));
	create_relationship("rId" + std::to_string(d_->relationships_.size() + 1), "xl/worksheets/sheet" + std::to_string(d_->worksheets_.size()) + ".xml", relationship::type::worksheet);
	
	return worksheet(&d_->worksheets_.back());
}

void workbook::add_sheet(xlnt::worksheet worksheet)
{
    for(auto ws : *this)
    {
        if(worksheet == ws)
        {
            throw std::runtime_error("worksheet already in workbook");
        }
    }
    
    d_->worksheets_.emplace_back(*worksheet.d_);
}

void workbook::add_sheet(xlnt::worksheet worksheet, std::size_t index)
{
    add_sheet(worksheet);
    std::swap(d_->worksheets_[index], d_->worksheets_.back());
}

int workbook::get_index(xlnt::worksheet worksheet)
{
    int i = 0;
    for(auto ws : *this)
    {
        if(worksheet == ws)
        {
            return i;
        }
        i++;
    }
    throw std::runtime_error("worksheet isn't owned by this workbook");
}

void workbook::create_named_range(const std::string &name, worksheet range_owner, const range_reference &reference)
{
    auto match = get_sheet_by_name(range_owner.get_title());
    if(match != nullptr)
    {
        match.create_named_range(name, reference);
        return;
    }
    throw std::runtime_error("worksheet isn't owned by this workbook");
}

void workbook::remove_named_range(const std::string &name)
{
    for(auto ws : *this)
    {
        if(ws.has_named_range(name))
        {
            ws.remove_named_range(name);
            return;
        }
    }
    
    throw std::runtime_error("named range not found");
}

range workbook::get_named_range(const std::string &name)
{
    for(auto ws : *this)
    {
        if(ws.has_named_range(name))
        {
            return ws.get_named_range(name);
        }
    }
    
    throw std::runtime_error("named range not found");
}

bool workbook::load(const std::istream &stream)
{
    std::string temp_file = create_temporary_filename();
    
    std::ofstream tmp;
    
    tmp.open(temp_file, std::ios::out | std::ios::binary);
    tmp << stream.rdbuf();
    tmp.close();
    
    load(temp_file);
    
    std::remove(temp_file.c_str());
    
    return true;
}
    
bool workbook::load(const std::vector<unsigned char> &data)
{
    xlnt::zip_file archive;
    archive.load(data);
    return load(archive);
}

bool workbook::load(const std::string &filename)
{
    zip_file f;

    try
    {
        f.load(filename);
    }
    catch(std::exception e)
    {
        throw invalid_file_exception(filename);
    }
    
    return load(f);
}
    
bool workbook::load(xlnt::zip_file &archive)
{
    auto content_types = read_content_types(archive);
    auto type = determine_document_type(content_types);

    if(type != "excel")
    {
        throw invalid_file_exception("");
    }
    
    clear();
    
    auto workbook_relationships = read_relationships(archive, "xl/workbook.xml");

    for(auto relationship : workbook_relationships)
    {
		create_relationship(relationship.get_id(), relationship.get_target_uri(), relationship.get_type());
    }
    
    pugi::xml_document doc;
    doc.load(archive.read("xl/workbook.xml").c_str());
    
    auto root_node = doc.child("workbook");
    
    auto workbook_pr_node = root_node.child("workbookPr");
    get_properties().excel_base_date = (workbook_pr_node.attribute("date1904") != nullptr && workbook_pr_node.attribute("date1904").as_int() != 0) ? calendar::mac_1904 : calendar::windows_1900;
    
    auto sheets_node = root_node.child("sheets");
    
    std::vector<std::string> shared_strings;
    
    if(archive.has_file("xl/sharedStrings.xml"))
    {
        shared_strings = read_shared_strings(archive.read("xl/sharedStrings.xml"));
    }

    std::vector<int> number_format_ids;
    std::unordered_map<int, std::string> custom_number_formats;
    
    if(archive.has_file("xl/styles.xml"))
    {
        pugi::xml_document styles_doc;
        styles_doc.load(archive.read("xl/styles.xml").c_str());
        auto stylesheet_node = styles_doc.child("styleSheet");
        auto cell_xfs_node = stylesheet_node.child("cellXfs");

        for(auto xf_node : cell_xfs_node.children("xf"))
        {
            number_format_ids.push_back(xf_node.attribute("numFmtId").as_int());
        }
        
        auto num_fmts_node = stylesheet_node.child("numFmts");
        
        for(auto num_fmt_node : num_fmts_node.children("numFmt"))
        {
            custom_number_formats[num_fmt_node.attribute("numFmtId").as_int()] = num_fmt_node.attribute("formatCode").as_string();
        }
    }
    
    for(auto sheet_node : sheets_node.children("sheet"))
    {
		std::string rel_id = sheet_node.attribute("r:id").as_string();
		auto rel = std::find_if(d_->relationships_.begin(), d_->relationships_.end(),
			[&](relationship &r) { return r.get_id() == rel_id; });

		if (rel == d_->relationships_.end())
		{
			throw std::runtime_error("relationship not found");
		}

        auto ws = create_sheet(sheet_node.attribute("name").as_string(), *rel);
        auto sheet_filename = rel->get_target_uri();

        read_worksheet(ws, archive.read(sheet_filename).c_str(), shared_strings, number_format_ids, custom_number_formats);
    }

    return true;
}

void workbook::set_guess_types(bool guess)
{
    d_->guess_types_ = guess;
}

bool workbook::get_guess_types() const
{
    return d_->guess_types_;
}

void workbook::create_relationship(const std::string &id, const std::string &target, relationship::type type)
{
    d_->relationships_.push_back(relationship(type, id, target));
}

relationship workbook::get_relationship(const std::string &id) const
{
    for(auto &rel : d_->relationships_)
    {
        if(rel.get_id() == id)
        {
            return rel;
        }
    }

    throw std::runtime_error("");
}
    
void workbook::remove_sheet(worksheet ws)
{
    auto match_iter = std::find_if(d_->worksheets_.begin(), d_->worksheets_.end(), [=](detail::worksheet_impl &comp) { return worksheet(&comp) == ws; });

    if(match_iter == d_->worksheets_.end())
    {
        throw std::runtime_error("worksheet not owned by this workbook");
    }

    
    d_->worksheets_.erase(match_iter);
}

worksheet workbook::create_sheet(std::size_t index)
{
    create_sheet();
    
	if (index != d_->worksheets_.size() - 1)
	{
		std::swap(d_->worksheets_.back(), d_->worksheets_[index]);
		d_->worksheets_.pop_back();
	}
    
    return worksheet(&d_->worksheets_[index]);
}

//TODO: There should be a better way to do this...
std::size_t workbook::index_from_ws_filename(const std::string &ws_filename)
{
    std::string sheet_index_string(ws_filename);
    sheet_index_string = sheet_index_string.substr(0, sheet_index_string.find('.'));
    sheet_index_string = sheet_index_string.substr(sheet_index_string.find_last_of('/'));
    auto iter = sheet_index_string.end();
    iter--;
    while (isdigit(*iter)) iter--;
    auto first_digit = static_cast<std::size_t>(iter - sheet_index_string.begin());
    sheet_index_string = sheet_index_string.substr(first_digit + 1);
    auto sheet_index = static_cast<std::size_t>(std::stoll(sheet_index_string) - 1);
    return sheet_index;
}
    
worksheet workbook::create_sheet(const std::string &title, const relationship &rel)
{
	d_->worksheets_.push_back(detail::worksheet_impl(this, title));

	auto index = index_from_ws_filename(rel.get_target_uri());
	if (index != d_->worksheets_.size() - 1)
	{
		std::swap(d_->worksheets_.back(), d_->worksheets_[index]);
		d_->worksheets_.pop_back();
	}

	return worksheet(&d_->worksheets_[index]);
}

worksheet workbook::create_sheet(std::size_t index, const std::string &title)
{
    auto ws = create_sheet(index);
    ws.set_title(title);
    
    return ws;
}

worksheet workbook::create_sheet(const std::string &title)
{
    if(title.length() > 31)
    {
        throw sheet_title_exception(title);
    }
    
    if(std::find_if(title.begin(), title.end(),
                    [](char c) { return c == '*' || c == ':' || c == '/' || c == '\\' || c == '?' || c == '[' || c == ']'; }) != title.end())
    {
        throw sheet_title_exception(title);
    }
    
    std::string unique_title = title;
    
    if(std::find_if(d_->worksheets_.begin(), d_->worksheets_.end(), [&](detail::worksheet_impl &ws) { return worksheet(&ws).get_title() == unique_title; }) != d_->worksheets_.end())
    {
        std::size_t suffix = 1;
        
        while(std::find_if(d_->worksheets_.begin(), d_->worksheets_.end(), [&](detail::worksheet_impl &ws) { return worksheet(&ws).get_title() == unique_title; }) != d_->worksheets_.end())
        {
            unique_title = title + std::to_string(suffix);
            suffix++;
        }
    }
    
    auto ws = create_sheet();
    ws.set_title(unique_title);

    return ws;
}

workbook::iterator workbook::begin()
{
    return iterator(*this, 0);
}

workbook::iterator workbook::end()
{
    return iterator(*this, d_->worksheets_.size());
}

workbook::const_iterator workbook::cbegin() const
{
    return const_iterator(*this, 0);
}

workbook::const_iterator workbook::cend() const
{
    return const_iterator(*this, d_->worksheets_.size());
}

std::vector<std::string> workbook::get_sheet_names() const
{
    std::vector<std::string> names;
    
    for(auto ws : *this)
    {
        names.push_back(ws.get_title());
    }
    
    return names;
}

worksheet workbook::operator[](const std::string &name)
{
    return get_sheet_by_name(name);
}

worksheet workbook::operator[](std::size_t index)
{
    return worksheet(&d_->worksheets_[index]);
}

void workbook::clear()
{
    d_->worksheets_.clear();
    d_->relationships_.clear();
    d_->active_sheet_index_ = 0;
    d_->drawings_.clear();
    d_->properties_ = document_properties();
}

bool workbook::save(std::vector<unsigned char> &data)
{
    data = save_virtual_workbook(*this);
    return true;
}

bool workbook::save(const std::string &filename)
{
    return save_workbook(*this, filename);
}

bool workbook::operator==(std::nullptr_t) const
{
    return d_.get() == nullptr;
}

bool workbook::operator==(const workbook &rhs) const
{
    return d_.get() == rhs.d_.get();
}

std::vector<relationship> xlnt::workbook::get_relationships() const
{
	return d_->relationships_;
}
 
std::vector<content_type> xlnt::workbook::get_content_types() const
{
	std::vector<content_type> content_types;
	content_types.push_back({ true, "xml", "", "application/xml" });
	content_types.push_back({ true, "rels", "", "application/vnd.openxmlformats-package.relationships+xml" });
	content_types.push_back({ false, "", "/xl/workbook.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml" });
	for(std::size_t i = 0; i < get_sheet_names().size(); i++)
	{
	    content_types.push_back({false, "", "/xl/worksheets/sheet" + std::to_string(i + 1) + ".xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"});
	}
	content_types.push_back({false, "", "/xl/theme/theme1.xml", "application/vnd.openxmlformats-officedocument.theme+xml"});
	content_types.push_back({false, "", "/xl/styles.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"});
	content_types.push_back({false, "", "/xl/sharedStrings.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"});
	content_types.push_back({false, "", "/docProps/core.xml", "application/vnd.openxmlformats-package.core-properties+xml"});
	content_types.push_back({false, "", "/docProps/app.xml", "application/vnd.openxmlformats-officedocument.extended-properties+xml"});
	return content_types;
}

document_properties &workbook::get_properties()
{
    return d_->properties_;
}

const document_properties &workbook::get_properties() const
{
    return d_->properties_;
}

void swap(workbook &left, workbook &right)
{
    using std::swap;
    swap(left.d_, right.d_);
    
    for(auto ws : left)
    {
        ws.set_parent(left);
    }
    
    for(auto ws : right)
    {
        ws.set_parent(right);
    }
}
    
workbook &workbook::operator=(workbook other)
{
    swap(*this, other);
    return *this;
}

workbook::workbook(workbook &&other) : workbook()
{
    swap(*this, other);
}
    
workbook::workbook(const workbook &other) : workbook()
{
    *d_.get() = *other.d_.get();
    
    for(auto ws : *this)
    {
        ws.set_parent(*this);
    }
}

bool workbook::get_data_only() const
{
    return d_->data_only_;
}

void workbook::set_data_only(bool data_only)
{
    d_->data_only_ = data_only;
}

void workbook::add_border(xlnt::border /*b*/)
{
    
}

void workbook::add_alignment(xlnt::alignment /*a*/)
{
    
}

void workbook::add_protection(xlnt::protection /*p*/)
{
    
}

void workbook::add_number_format(const std::string &/*format*/)
{
    
}

void workbook::add_fill(xlnt::fill &/*f*/)
{
    
}

void workbook::add_font(xlnt::font /*f*/)
{
    
}

void workbook::set_code_name(const std::string &/*code_name*/)
{
    
}

bool workbook::has_loaded_theme()
{
    return false;
}

std::string workbook::get_loaded_theme()
{
    return "";
}

std::vector<named_range> workbook::get_named_ranges() const
{
    std::vector<named_range> named_ranges;
    
    for(auto ws : *this)
    {
        for(auto &ws_named_range : ws.d_->named_ranges_)
        {
            named_ranges.push_back(ws_named_range.second);
        }
    }
    
    return named_ranges;
}

std::size_t workbook::add_style(xlnt::style style_)
{
    return 1;
}

const number_format &workbook::get_number_format(std::size_t style_id) const
{
    return d_->number_formats_[d_->styles_[style_id].number_format_index_];
}

const font &workbook::get_font(std::size_t style_id) const
{
    return d_->fonts_[d_->styles_[style_id].font_index_];
}

std::size_t workbook::set_font(const font &font_, std::size_t style_id)
{
    auto match = std::find(d_->fonts_.begin(), d_->fonts_.end(), font_);
    auto font_index = 0;
    
    if(match == d_->fonts_.end())
    {
        d_->fonts_.push_back(font_);
        font_index = d_->fonts_.size() - 1;
    }
    else
    {
        font_index = match - d_->fonts_.begin();
    }
    
    auto existing_style = d_->styles_[style_id];
    
    if(font_index == existing_style.font_index_)
    {
        // no change
        return style_id;
    }
    
    auto new_style = existing_style;
    new_style.font_index_ = font_index;
    
    auto style_match = std::find(d_->styles_.begin(), d_->styles_.end(), new_style);

    if(style_match != d_->styles_.end())
    {
        return style_match - d_->styles_.begin();
    }
    
    d_->styles_.push_back(new_style);
    
    return d_->styles_.size() - 1;
}

const fill &workbook::get_fill(std::size_t style_id) const
{
    return d_->fills_[d_->styles_[style_id].fill_index_];
}

std::size_t workbook::set_fill(const fill &fill_, std::size_t style_id)
{
    return style_id;
}

const border &workbook::get_border(std::size_t style_id) const
{
    return d_->borders_[d_->styles_[style_id].border_index_];
}

std::size_t workbook::set_border(const border &border_, std::size_t style_id)
{
    return style_id;
}

const alignment &workbook::get_alignment(std::size_t style_id) const
{
    return d_->alignments_[d_->styles_[style_id].alignment_index_];
}

std::size_t workbook::set_alignment(const alignment &alignment_, std::size_t style_id)
{
    return style_id;
}

const protection &workbook::get_protection(std::size_t style_id) const
{
    return d_->protections_[d_->styles_[style_id].number_format_index_];
}

std::size_t workbook::set_protection(const protection &protection_, std::size_t style_id)
{
    return style_id;
}

bool workbook::get_pivot_button(std::size_t style_id) const
{
    return d_->styles_[style_id].pivot_button_;
}

bool workbook::get_quote_prefix(std::size_t style_id) const
{
    return d_->styles_[style_id].quote_prefix_;
}

std::size_t workbook::set_number_format(const xlnt::number_format &format, std::size_t style_id)
{
    auto match = std::find(d_->number_formats_.begin(), d_->number_formats_.end(), format);
    auto format_index = 0;
    
    if(match == d_->number_formats_.end())
    {
        d_->number_formats_.push_back(format);
        format_index = d_->number_formats_.size() - 1;
    }
    else
    {
        format_index = match - d_->number_formats_.begin();
    }
    
    if(d_->styles_.empty())
    {
        style new_style;
        new_style.style_index_ = 0;
        new_style.alignment_index_ = 0;
        new_style.border_index_ = 0;
        new_style.fill_index_ = 0;
        new_style.font_index_ = 0;
        new_style.number_format_index_ = format_index;
        new_style.protection_index_ = 0;
        d_->styles_.push_back(new_style);
        return 0;
    }
    
    auto existing_style = d_->styles_[style_id];
    
    if(format_index == existing_style.number_format_index_)
    {
        // no change
        return style_id;
    }
    
    auto new_style = existing_style;
    new_style.number_format_index_ = format_index;
    new_style.number_format_ = format;
    
    auto style_match = std::find(d_->styles_.begin(), d_->styles_.end(), new_style);
    
    if(style_match != d_->styles_.end())
    {
        return style_match - d_->styles_.begin();
    }
    
    d_->styles_.push_back(new_style);
    
    return d_->styles_.size() - 1;
}
    
std::vector<style> workbook::get_styles() const
{
    return d_->styles_;
}

std::vector<number_format> workbook::get_number_formats() const
{
    return d_->number_formats_;
}

std::vector<font> workbook::get_fonts() const
{
    return d_->fonts_;
}
    
} // namespace xlnt
