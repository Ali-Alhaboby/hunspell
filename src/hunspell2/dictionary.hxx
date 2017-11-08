/* Copyright 2016-2017 Dimitrij Mijoski
 *
 * This file is part of Hunspell.
 *
 * Hunspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hunspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Hunspell.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aff_data.hxx"
#include "dic_data.hxx"

#include <boost/locale.hpp>
#include <fstream>
#include <locale>

namespace hunspell {

enum Spell_result {
	bad_word,
	good_word,
	affixed_good_word,
	compound_good_word
};

class Dictionary {
	using string = std::string;
	using u16string = std::u16string;

	Aff_data aff_data;
	Dic_data dic_data;

      private:
	/* (0)
	All the major work is done here.
	(1) and (2) are the lowest level specializations.
	The rest just do some conversions and delegate to them.

	(1) will simply call this
	with ConversionIterator set to string::iterator

	(2) will call this with u8_u32 on the fly conversion iterator.
	*/
	template <class ConvIter>
	auto spell(ConvIter start, ConvIter end, const string& s)
	    -> Spell_result;

	/*
	 * (1) This should be called when the input and the dictionary
	 * are in the same encoding and that encoding is single byte encoding.
	 */
	auto spell_singlechar_input_singlechar_dict(const string& word)
	    -> Spell_result;

	/*
	 * (2) This should be called when the input and the dictionary
	 * are in the same encoding and that encoding UTF-8.
	 */
	auto spell_u8_input_u8_dict(const string& word) -> Spell_result;

      public:
	Dictionary()
	    : // we explicity do value init so content is zeroed
	      aff_data(),
	      dic_data()
	{
	}
	explicit Dictionary(const string& dict_file_path)
	    : aff_data(), dic_data()
	{
		std::ifstream aff_file(dict_file_path + ".aff");
		std::ifstream dic_file(dict_file_path + ".dic");
		aff_data.parse(aff_file);
		dic_data.parse(dic_file, aff_data);
	}

	auto spell_dict_encoding(const std::string& word) -> Spell_result;

	auto spell_c_locale(const std::string& word) -> Spell_result;

	auto spell(const std::string& word, std::locale loc = std::locale())
	    -> Spell_result
	{
		if (std::has_facet<boost::locale::info>(loc)) {

			auto& fac = std::use_facet<boost::locale::info>(loc);
			auto from_enc = fac.encoding();
			auto& to_enc = aff_data.encoding;
			auto w = boost::locale::conv::between(word, to_enc,
			                                      from_enc);
			// this just for mocking ATM
			if (dic_data.words.count(w))
				return good_word;
			return bad_word;
		}
		std::cerr << "No info facet in locale" << std::endl;
		return bad_word;
	}
	auto spell_u8(const std::string& word) -> Spell_result;
	auto spell(const std::wstring& word) -> Spell_result;
	auto spell(const std::u16string& word) -> Spell_result;
	auto spell(const std::u32string& word) -> Spell_result;
};
}
