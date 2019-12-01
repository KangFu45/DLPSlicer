#ifndef ICONFIG_H
#define ICONFIG_H

#include "XmlConfig.h"

namespace jdb{
	/* Interface for classes using an XmlConfig
	 * Provides an XmlConfig object and nodePath variable
	 */
	class IConfig
	{
	public:

		virtual ~IConfig() {}
	protected:
		// the XmlConfig for this object
		XmlConfig config;

		// path to this objects data if applicable
		string nodePath;
	};	
}


#endif