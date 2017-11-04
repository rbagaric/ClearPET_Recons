/// \file	options.cpp
/// \brief	Implementation of class OptionsParser
/// \author	Axel Wagner

#include <cstring>
#include "options.h"

OptionsParser::OptionsParser(int argc, char** argv)
{
	using namespace std;

	for (int iArg = 1; iArg < argc; ++iArg)
	{
		if (argv[iArg][0] == '-' || argv[iArg][0] == '/')
		{
			// is an option
			char* p = argv[iArg]+1;
			if (argv[iArg][0] == '-' && *p == '-')
				// the "--" option case
				++p;
			char* q = strchr(p, '=');
			if (q)
			{
				// is an option with value "-opt=value"
				string opt(p, q-p);
				string value(q+1);
				options[opt] = value;
			} else
			{
				// is an empty option "-opt"
				string opt(p);
				options[opt] = string();
			}
		} else
		{
			// is an ordinary argument
			arguments.push_back(argv[iArg]);
		}
	}
}

///\brief Return one of the non-option arguments
///
///\param iArg The index of the argument, in range 0..GetArgumentCount()-1
///\return the argument, or empty string if the index is out of range
///
std::string OptionsParser::GetArgument(unsigned int iArg) const
{
	if (iArg >= arguments.size())
		return std::string();
	else
		return arguments[iArg];
}

///\brief Find an option in the list and return an iterator to it.
///
/// The option name(s) are given in a string. Several optional names
/// are allowes, separated by the '|' character. For example, "o|out"
/// will return the first option with name "o" or "out".
///
/// This is a helper function for GetOption() and HasOption().
///
OptionsParser::options_list::const_iterator OptionsParser::FindOption(const char* optNames) const
{
	using namespace std;

	const char* p = optNames;
	string opt;

	while (p && *p)
	{
		const char* q = strchr(p, '|');
		if (!q)
		{
			opt = p;
			p = 0;
		} else
		{
			opt = string(p, q-p);
			p = q+1;
		}

		options_list::const_iterator i = options.find(opt);
		if (i != options.end())
			return i;
	}

	// not found
	return options.end();
}


///\brief Find an option and return the value
///
/// The option name(s) are given in a string. Several optional names
/// are allowes, separated by the '|' character. For example, "o|out"
/// will return the first option with name "o" or "out".
///
std::string OptionsParser::GetOption(const char* optNames) const
{
	options_list::const_iterator i = FindOption(optNames);
	if (i != options.end())
		return i->second;
	else
		return std::string();
}

///\brief Check if an option was given
///
/// The option name(s) are given in a string. Several optional names
/// are allowes, separated by the '|' character. For example, "o|out"
/// will return the first option with name "o" or "out".
///
/// This is quite the same as GetOption(), except that we do not return
/// the option value. This function is useful for option which do not have
/// a value.
///
bool OptionsParser::HasOption(const char* optNames) const
{
	return options.end() != FindOption(optNames);
}

