//@	{"targets":[{"name":"text2native","type":"application"}]}

#include "alice/alice.hpp"
#include "analib/typeset/type_set.hpp"

#include <cstring>
#include <algorithm>
#include <cstdint>
#include <clocale>

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

template<int N>
class FixedBuffer
	{
	public:
		FixedBuffer():m_insert_pos(0), m_data{} {}

		void push_back(char ch_in)
			{
			if(m_insert_pos >= N)
				{
				Alice::ErrorMessage msg{};
				sprintf(msg.data, "Token too long: %d (%d)", m_insert_pos, N);
				throw msg;
				}
			m_data[m_insert_pos] = ch_in;
			++m_insert_pos;
			}

		void clear()
			{
			m_insert_pos=0;
			m_data[0] = '\0';
			}

		char const* c_str() const
			{return m_data.data();}

		static constexpr auto npos = N;

	private:
		int m_insert_pos;
		std::array<char, N> m_data;
	};

template<class BufferType>
[[nodiscard]] bool readData(FILE* src, BufferType& buffer)
	{
	int ch_in{};
	while(true) // Eat whitepsace
		{
		ch_in = getc(src);
		if(ch_in==EOF)
			{return false;}
		if(!(ch_in>='\0' && ch_in<=' '))
			{break;}
		}

	buffer.push_back(ch_in);
	while(true)
		{
		ch_in = getc(src);

		if((ch_in>='\0' && ch_in<=' ') || ch_in==EOF)
			{
			buffer.push_back('\0');
			return true;
			}

		buffer.push_back(ch_in);
		}
	}

template<class BufferType>
[[noreturn]] void throwOverflowError(BufferType const& buffer)
	{
	Alice::ErrorMessage msg{};
	static_assert(BufferType::npos + 32 <= sizeof(msg));
	sprintf(msg.data, "Overflow or bad input data: %s", buffer.c_str());
	throw msg;
	}

template<class T, class BufferType, class Converter>
[[nodiscard]] auto convert(BufferType const& buffer, Converter&& conv)
	{
	char* ptr{};
	errno = 0;
	if constexpr(std::is_integral_v<T>)
		{
		auto val = conv(buffer.c_str(), &ptr, 10);

		if(*ptr != '\0' || errno == EINVAL || errno == ERANGE
			|| val > static_cast<decltype(val)>(std::numeric_limits<T>::max())
			|| val < static_cast<decltype(val)>(std::numeric_limits<T>::min()))
			{throwOverflowError(buffer);}

		return static_cast<T>(val);
		}
	else
	if constexpr(std::is_floating_point_v<T>)
		{
		auto val = conv(buffer.c_str(), &ptr);

		if(*ptr != '\0' || errno == EINVAL || errno == ERANGE)
			{throwOverflowError(buffer);}

		return val;
		}
	}

class Action
	{
	public:
		Action(std::string const& locale)
			{setlocale(LC_NUMERIC, locale.c_str());}

		void operator()(){}

		template<class T>
		void operator()(Analib::Empty<T>)
			{
			FixedBuffer<28> buffer;
			while(readData(stdin, buffer))
				{
				if constexpr(std::is_integral_v<T> && std::is_unsigned_v<T>)
					{
					if(*buffer.c_str()=='-')
						{throwOverflowError(buffer);}

					auto val = convert<T>(buffer, strtoull);
					fwrite(&val, sizeof(val), 1, stdout);
					}
				else
				if constexpr(std::is_integral_v<T> && !std::is_unsigned_v<T>)
					{
					auto val = convert<T>(buffer,  strtoll);
					fwrite(&val, sizeof(val), 1, stdout);
					}
				else
				if constexpr(std::is_same_v<T, float>)
					{
					auto val = convert<T>(buffer, strtof);
					fwrite(&val, sizeof(val), 1, stdout);
					}
				else
				if constexpr(std::is_same_v<T, double>)
					{
					auto val = convert<T>(buffer, strtod);
					fwrite(&val, sizeof(val), 1, stdout);
					}
				buffer.clear();
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
