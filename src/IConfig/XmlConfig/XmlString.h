#ifndef XML_STRING_H
#define XML_STRING_H


#include "Utils.h"

#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <string>
#include <map>
#include <cstdlib>

#include "XmlConfig.h"


using namespace std;

// Formats a string like
// "this is a {key1} and {path.to.node}"
namespace jdb{

	class XmlConfig;

	class XmlString
	{
	public:
		virtual const char* classname() const { return "XmlString"; }
		XmlString();
		~XmlString();


		static string TOKEN_START;
		static string TOKEN_STOP;

		void add( string k, string v ){
			//DEBUG( classname(), "Adding [" << k << "] = " << v );
			kv[ k ] = v;
		}
		void add( string k, int v ){
			add( k, ts(v) );
		}
		void add( string k, double v ){
			add( k, dts(v) );
		}
		void add( string k, float v ){
			add( k, dts(v) );
		}

		string first_token_at( string &s, int &index, int &len, int pos = 0 ) const {
			// DEBUG( classname(), s << ", index=" << index << ", len=" << len << ", pos=" << pos );
			string rs = s;
				
			string &_tkstart = XmlString::TOKEN_START;
			string &_tkstop = XmlString::TOKEN_STOP;

			index = s.find( _tkstart, pos );
			string test;
			test.push_back(s[index+1]);
			while( _tkstart == test && index != string::npos ){
				int nindex = s.find( _tkstart, index+3 );
				// DEBUG( classname(), "escaped {{ @" << index );
				if ( nindex < index ){
					index = nindex;
					break;
				}
				index = nindex;
			}

			int stop = s.find( _tkstop, index >= pos ? index : pos);
			len = stop - index + 1;
			
			if ( -1 == index || -1 == stop || len < 0 ) return rs;
			
			rs = s.substr( index+1, len-2 ); // just the part inside of the {}
			return rs;
		}

		bool replace_token( const XmlConfig &cfg, string &_s, string &_key, int index, int len, bool _clobber = false );
		bool replace_token( string &_s, string &_key, int index, int len, bool _clobber = false );

		string format( const XmlConfig &_cfg, string _s, bool _clobber = false ) {
			int index = -1;
			int len = -1;
			int pos = 0;
			string key = first_token_at( _s, index, len, pos );
			// DEBUG( classname(), "key = " << key );
			while ( index >= 0 ){

				// DEBUG( classname(), "now : " << _s );
				bool replaced = replace_token( _cfg, _s, key, index, len, _clobber );
				// DEBUG( classname(), "new : " << _s );

				pos = index;
				if ( false == replaced ) pos++;
				// DEBUG( classname(), "pos = " << pos );
				// DEBUG( classname(), key );
				key = first_token_at( _s, index, len, pos );
				// DEBUG( classname(), "key = " << key );
			}
			unescape(_s);
			return _s;
		}

		string format( string _s, bool _clobber = false ) {
			int index = -1;
			int len = -1;
			int pos = 0;
			string key = first_token_at( _s, index, len, pos );
			while ( index >= 0 ){

				bool replaced = replace_token( _s, key, index, len, _clobber );
				// DEBUG( classname(), "new : " << _s );

				pos = index;
				if ( false == replaced ) pos++;
				// DEBUG( classname(), "pos = " << pos );
				// DEBUG( classname(), key );
				key = first_token_at( _s, index, len, pos );
			}
			unescape(_s);
			return _s;
		}

		void unescape( string &_s ){
			int index = _s.find( "{{", 0 );
			while( index >= 0 ){
				_s.replace( index, 1, "" );
				index = _s.find( "{{", index+1 );
			}

			index = _s.find( "}}", 0 );
			while( index >= 0 ){
				_s.replace( index, 1, "" );
				index = _s.find( "}}", index+1 );
			}
		}

		bool hasTokens( string _s ){
			int index = -1;
			int len = -1;
			int pos = 0;
			string key = first_token_at( _s, index, len, pos );
			if ( key == _s )
				return false;
			return true;
		}

		vector<string> tokens( string _s, int n = -1 ) {
			int index = -1;
			int len = -1;
			int pos = 0;
			vector<string> tks;
			string key = first_token_at( _s, index, len, pos );
			int iToken = 1;
			while ( index >= 0 ){
				tks.push_back( key );
				if ( n >= 1 && iToken >= n ) return tks;
				
				_s.replace( index, len, "" );
				// DEBUG( classname(), "new : " << _s );

				pos = index;
				// DEBUG( classname(), "pos = " << pos );
				// DEBUG( classname(), key );
				key = first_token_at( _s, index, len, pos );
				iToken ++;
			}
			return tks;
		}

		string clean( string _s, int n = -1 ) {
			int index = -1;
			int len = -1;
			int pos = 0;
			string key = first_token_at( _s, index, len, pos );
			int iToken = 1;
			while ( index >= 0 ){

				_s.replace( index, len, "" );
				// DEBUG( classname(), "new : " << _s );

				pos = index;
				// DEBUG( classname(), "pos = " << pos );
				// DEBUG( classname(), key );
				key = first_token_at( _s, index, len, pos );
				if ( n >= 1 && iToken >= n ) return _s;
				iToken ++;
			}
			unescape(_s);
			return _s;
		}


	protected:
		map<string, string> kv;

		
	};
}


#endif