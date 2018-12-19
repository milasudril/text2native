//@	{"targets":[{"name":"native2text","type":"application"}]}

#include "alice/alice.hpp"
#include "analib/typeset/type_set.hpp"

#include <cstring>
#include <algorithm>
#include <cstdint>
#include <clocale>
#include <inttypes.h>

ALICE_OPTION_DESCRIPTOR(OptionDescriptor
	,{"General" ,"help","Print usage information","string",Alice::Option::Multiplicity::ZERO_OR_ONE}
	,{"Input options","type","Data type to decode","type identifier",Alice::Option::Multiplicity::ONE}
	,{"Input options","locale", "The locale used to decode floating point numberes. By default, it is set to the LC_NUMERIC value of the parent process.", "string", Alice::Option::Multiplicity::ONE}
	);

namespace Alice
	{
	template<>
	struct MakeType<Stringkey("type identifier")>:public MakeType<Stringkey("string")>
		{
		static constexpr char const* descriptionShortGet() noexcept
			{return "i8 | i16 | i32 | i64 | u8 | u16 | u32 | u64 | f32 | f64";}

		static constexpr char const* descriptionLongGet() noexcept
			{
			return "";
			}
		};
	}

static constexpr char const* type_names[]={"i8","i16","i32","i64","u8","u16","u32","u64","f32","f64"};
using SupportedTypes = Analib::TypeSet
	<
	 int8_t
	,int16_t
	,int32_t
	,int64_t
	,uint8_t
	,uint16_t
	,uint32_t
	,uint64_t
	,float
	,double
	>;

class Action
	{
	public:
		Action(std::string const& locale)
			{setlocale(LC_NUMERIC, locale.c_str());}

		void operator()(){}

		template<class T>
		void operator()(Analib::Empty<T>)
			{
			T value{};
			while(fread(&value, sizeof(value), 1, stdin))
				{
				if constexpr(std::is_integral_v<T> && std::is_unsigned_v<T>)
					{printf("%" PRIu64 " ", static_cast<uint64_t>(value));}
				else
				if constexpr(std::is_integral_v<T> && !std::is_unsigned_v<T>)
					{printf("%" PRId64 " ", static_cast<int64_t>(value));}

				else
				if constexpr(std::is_same_v<T, float>)
					{printf("%.9e ", static_cast<double>(value));}
				else
				if constexpr(std::is_same_v<T, double>)
					{printf("%.17e ", value);}
				}
			}
	};

size_t mapTypeId(std::string const& str)
	{
	auto i = std::find_if(std::begin(type_names), std::end(type_names),[&str](char const* val)
		{return strcmp(val, str.c_str())==0;});

	if(i == std::end(type_names))
		{
		Alice::ErrorMessage msg{};
		sprintf(msg.data, "Unknown type %s. Try --help form more information.", str.c_str());
		throw msg;
		}
	return i - std::begin(type_names);
	}

int main(int argc, char* argv[])
	{
	try
		{
		Alice::CommandLine<OptionDescriptor> cmd_line{argc, argv};

		if(cmd_line.get<Alice::Stringkey("help")>())
			{
			cmd_line.help(1);
			return 0;
			}

		SupportedTypes::select
			(
			 mapTypeId(cmd_line.get<Alice::Stringkey("type")>().valueGet())
			,Action{cmd_line.get<Alice::Stringkey("locale")>().valueGet()}
			);

		return 0;
		}
	catch(const Alice::ErrorMessage& message)
		{
		fprintf(stderr,"%s\n",message.data);
		return -1;
		}
	}
