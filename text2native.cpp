//@	{"targets":[{"name":"text2native","type":"application"}]}

#include "alice/alice.hpp"
#include "analib/typeset/type_set.hpp"

#include <cstring>
#include <algorithm>
#include <cstdint>

ALICE_OPTION_DESCRIPTOR(OptionDescriptor
	,{"General" ,"help","Print usage information","string",Alice::Option::Multiplicity::ZERO_OR_ONE}
	,{"Input options","type","Data type to decode","type identifier",Alice::Option::Multiplicity::ONE}
	,{"Input options","locale", "The locale used to decode floating point numberes", "string", Alice::Option::Multiplicity::ONE}
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
		Action(std::string const& locale) {}

		void operator()(){}

		void operator()(Analib::Empty<int8_t>) {}
		void operator()(Analib::Empty<int16_t>) {}
		void operator()(Analib::Empty<int32_t>) {}
		void operator()(Analib::Empty<int64_t>) {}

		void operator()(Analib::Empty<uint8_t>) {}
		void operator()(Analib::Empty<uint16_t>) {}
		void operator()(Analib::Empty<uint32_t>) {}
		void operator()(Analib::Empty<uint64_t>) {}

		void operator()(Analib::Empty<float>) {}
		void operator()(Analib::Empty<double>) {}
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
