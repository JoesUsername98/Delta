#pragma once

//******************************************************
//*****            COPIED FILES            *************
#include "magic_enum/magic_enum.hpp"
//*****   ~~~~~~~  COPIED FILES   ~~~~~~  **************
//******************************************************

template <typename T>
class EnumCombo
{
public:
	const std::array<std::string_view, magic_enum::enum_count<T>()> m_keys;
	const char* m_keysCArray[magic_enum::enum_count<T>()];
	EnumCombo()
		: m_keys{ magic_enum::enum_names<T>() }
	{
		for (size_t i = 0; i < m_keys.size(); ++i)
			m_keysCArray[i] = m_keys[i].data();
	}
};