#ifndef IOBJECT_H
#define IOBJECT_H

namespace jdb {
	/* Generic Object Interface
	 * Provides basic functionality to objects
	 */
	class IObject
	{
	public:
		virtual ~IObject() {};
		/* Classname
		 * Used for identifying classes. Subclasses should override 
		 *
		 * @return "Classname"
		 */
		virtual const char* classname() const = 0;
	};
}


#endif