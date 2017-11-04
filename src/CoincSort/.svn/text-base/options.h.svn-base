/// \file	options.h
/// \brief	Declaration of class OptionsParser
/// \author	Axel Wagner
///
#ifndef options_h_included
#define options_h_included

#include <string>
#include <vector>
#include <map>

///\brief Command line options parser
///
/// This class parses command line arguments for options and stores
/// them for easier lookup.
///
/// The parser detects options on the starting character which must be
/// '-' or '/'. "--" is also possible. All other arguments are stored
/// as ordinary arguments in list of appearance.
///
/// Options can be empty or can have values. Values must follow the
/// option name separated by a '=' character. Example: -opt=value.
///
class OptionsParser
{
public:
	OptionsParser(int argc, char** argv);

	/// return the number of ordinary arguments (without option prefix)
	int GetArgumentCount() const
	{
		return arguments.size();
	}

	/// return the number of option arguments
	int GetOptionsCount() const
	{
		return options.size();
	}

	std::string GetArgument(unsigned int iArg) const;
	std::string GetOption(const char* optNames) const;
	bool		HasOption(const char* optNames) const;

private:
	typedef std::vector<std::string>	arguments_list;
	typedef std::map<std::string, std::string>	options_list;

	arguments_list	arguments;
	options_list	options;

	options_list::const_iterator FindOption(const char* optNames) const;
};

#endif // options_h_included
