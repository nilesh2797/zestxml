#pragma once

#include <unordered_map>
#include <sstream>
#include <typeinfo>

using namespace std;

class Parameters 
{
public:

	template<typename ParamType>
	inline void set(const std::string &name, const ParamType &value) 
	{
		std::stringstream sstream;
		sstream << value;
		params[name] = sstream.str();
	}

	template<typename ParamType>
	inline ParamType get(const std::string &name) const 
	{
		auto item = params.find(name);
		if (item == params.end())
		{
			string throw_string = string("Invalid parameter name : ") + name;
			throw std::invalid_argument(throw_string);
		}
		else
			return ConvertStrToValue<ParamType>(item->second);
	}

	template<typename ParamType>
	inline ParamType get(const std::string &name, const ParamType &default_value) 
	{
		try
		{
			return get<ParamType>(name);
		}
		catch (std::invalid_argument e)
		{
			return default_value;
		}
	}

	void dump(string fname)
	{
		ofstream fout(fname);
		for(auto& it : params)
		{
			fout << it.first << " " << it.second << endl;
		}
		fout.close();
	}

	void print()
	{
		for(auto& it : params)
		{
			cout << it.first << " " << it.second << endl;
		}
	}

private:

  std::unordered_map<std::string, std::string> params;

	template<typename ParamType>
	inline ParamType ConvertStrToValue(const std::string &str) const 
	{
		std::stringstream sstream(str);
		ParamType value;

		if (!(sstream >> value) || !sstream.eof()) 
		{
			std::stringstream err;
			err << "Failed to convert value '" << str << "' to type: " << typeid(value).name();
			throw std::runtime_error(err.str());
		}

		return value;
	}


};