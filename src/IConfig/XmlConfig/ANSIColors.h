#ifndef ANSI_COLORS_H
#define ANSI_COLORS_H

#include <sstream>
#include <string>

namespace jdb{
	class ANSIColors
	{
	public:
		ANSIColors() {}
		~ANSIColors() {}
	
		static const int BOLD = 1;
		static const int RED = 31;
		static const int GREEN = 32;
		static const int YELLOW = 33;
		static const int BLUE = 34;
		static const int FG_DEFAULT = 39;

		static void modify( std::string &_str, int _code = 31, bool brighten = true ){
			std::stringstream sstr; 
			sstr << "\033[" << _code << "m" << _str << ANSIColors::reset();
			_str = sstr.str();
		}

		static std::string reset(){
			return "\033[0;m";
		}

		static std::string color( std::string _str, std::string _color, bool bright = true ){
			std::string _rstr = _str;
			modify( _rstr, ANSIColors::fg_color_from_string( _color, bright ) );
			return _rstr;
		}

		static std::string bold( std::string _str ){
			std::string _rstr = _str;
			modify( _rstr, BOLD );
			return _rstr;
		}

		static int fg_color_from_string( std::string _color, bool brighten = true ){
			int brightMod = 0;
			if ( brighten ) brightMod = 60;
			if ( "red" == _color || "Red" == _color || "RED" == _color )
				return ANSIColors::RED + brightMod;
			if ( "blue" == _color || "Blue" == _color || "BLUE" == _color )
				return ANSIColors::BLUE + brightMod;
			if ( "yellow" == _color || "Yellow" == _color || "YELLOW" == _color )
				return ANSIColors::YELLOW + brightMod;
			if ( "green" == _color || "Green" == _color || "GREEN" == _color )
				return ANSIColors::GREEN + brightMod;
			return ANSIColors::FG_DEFAULT;
		}


	};
}
#endif