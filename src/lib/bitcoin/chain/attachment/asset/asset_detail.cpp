/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/chain/attachment/asset/asset_detail.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin {
namespace chain {

asset_detail::asset_detail()
{
    reset();
}
asset_detail::asset_detail(
    std::string symbol, uint64_t maximum_supply,
    uint8_t decimal_number, uint8_t threshold, std::string issuer,
    std::string address, std::string description):
    symbol(symbol), maximum_supply(maximum_supply),
    decimal_number(decimal_number),
    secondaryissue_threshold(threshold),
    attenuation_model_index(ATTENUATION_MODEL_NONE),
    unused2(0), unused3(0),
    issuer(issuer), address(address), description(description)
{
}

asset_detail asset_detail::factory_from_data(const data_chunk& data)
{
    asset_detail instance;
    instance.from_data(data);
    return instance;
}

asset_detail asset_detail::factory_from_data(std::istream& stream)
{
    asset_detail instance;
    instance.from_data(stream);
    return instance;
}

asset_detail asset_detail::factory_from_data(reader& source)
{
    asset_detail instance;
    instance.from_data(source);
    return instance;
}
bool asset_detail::is_valid() const
{
    return !(symbol.empty()
            || (maximum_supply==0)
            || (symbol.size() + 8 + 4 + issuer.size() + address.size() + description.size() + 4)>ASSET_DETAIL_FIX_SIZE);
}

void asset_detail::reset()
{
    symbol = "";
    maximum_supply = 0;
    decimal_number = 0;
    secondaryissue_threshold = 0;
    attenuation_model_index = ATTENUATION_MODEL_NONE;
    unused2 = 0;
    unused3 = 0;
    issuer = "";
    address = "";
    description = "";
}

bool asset_detail::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool asset_detail::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool asset_detail::from_data(reader& source)
{
    reset();

    symbol = source.read_string();
    maximum_supply = source.read_8_bytes_little_endian();
    decimal_number = source.read_byte();
    secondaryissue_threshold = source.read_byte();
    uint8_t byte_num = source.read_byte();
    attenuation_model_index = (byte_num & 0x7); // lower 3 bits
    unused2 = ((byte_num >> 3) & 0x1f); // higher 5 bits
    unused3 = source.read_byte();
    issuer = source.read_string();
    address =  source.read_string();
    description =  source.read_string();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk asset_detail::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void asset_detail::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void asset_detail::to_data(writer& sink) const
{
    sink.write_string(symbol);
    sink.write_8_bytes_little_endian(maximum_supply);
    sink.write_byte(decimal_number);
    sink.write_byte(secondaryissue_threshold);
    uint8_t byte_num = (attenuation_model_index + (unused2 << 3));
    sink.write_byte(byte_num);
    sink.write_byte(unused3);
    sink.write_string(issuer);
    sink.write_string(address);
    sink.write_string(description);
}

uint64_t asset_detail::serialized_size() const
{
    size_t len = symbol.size() + 8 + 4 + issuer.size() + address.size() + description.size() + 4;
    return std::min(ASSET_DETAIL_FIX_SIZE, len);
}

std::string asset_detail::to_string() const
{
    std::ostringstream ss;

    ss << "\t symbol = " << symbol << "\n"
        << "\t maximum_supply = " << std::to_string(maximum_supply) << "\n"
        << "\t decimal_number = " << std::to_string(decimal_number) << "\n"
        << "\t is_asset_secondaryissue = " << (is_asset_secondaryissue() ? "true" : "false") << "\n"
        << "\t secondaryissue_threshold = " << std::to_string(get_secondaryissue_threshold()) << "\n"
        << "\t attenuation_model_index = " << std::to_string(get_attenuation_model_index()) << "\n"
        << "\t issuer = " << issuer << "\n"
        << "\t address = " << address << "\n"
        << "\t description = " << description << "\n";

    return ss.str();
}

const std::string& asset_detail::get_symbol() const
{
    return symbol;
}
void asset_detail::set_symbol(const std::string& symbol)
{
    size_t len = symbol.size()+1 < (ASSET_DETAIL_SYMBOL_FIX_SIZE) ?symbol.size()+1:ASSET_DETAIL_SYMBOL_FIX_SIZE;
    this->symbol = symbol.substr(0, len);
}

uint64_t asset_detail::get_maximum_supply() const
{
    return maximum_supply;
}
void asset_detail::set_maximum_supply(uint64_t maximum_supply)
{
     this->maximum_supply = maximum_supply;
}

uint8_t asset_detail::get_decimal_number() const
{
    return decimal_number;
}
void asset_detail::set_decimal_number(uint8_t decimal_number)
{
     this->decimal_number = decimal_number;
}

const std::string& asset_detail::get_issuer() const
{
    return issuer;
}
void asset_detail::set_issuer(const std::string& issuer)
{
     size_t len = issuer.size()+1 < (ASSET_DETAIL_ISSUER_FIX_SIZE) ?issuer.size()+1:ASSET_DETAIL_ISSUER_FIX_SIZE;
     this->issuer = issuer.substr(0, len);
}

const std::string& asset_detail::get_address() const
{
    return address;
}
void asset_detail::set_address(const std::string& address)
{
     size_t len = address.size()+1 < (ASSET_DETAIL_ADDRESS_FIX_SIZE) ?address.size()+1:ASSET_DETAIL_ADDRESS_FIX_SIZE;
     this->address = address.substr(0, len);
}

const std::string& asset_detail::get_description() const
{
    return description;
}
void asset_detail::set_description(const std::string& description)
{
     size_t len = description.size()+1 < (ASSET_DETAIL_DESCRIPTION_FIX_SIZE) ?description.size()+1:ASSET_DETAIL_DESCRIPTION_FIX_SIZE;
     this->description = description.substr(0, len);
}

asset_cert_type asset_detail::get_asset_cert_mask() const
{
    auto certs = asset_cert_ns::none;
    if (is_secondaryissue_legal()) {
        certs |= asset_cert_ns::issue;
    }
    return certs;
}

} // namspace chain
} // namspace libbitcoin
