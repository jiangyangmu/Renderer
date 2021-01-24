#pragma once

#include "Common.h"

namespace Graphics
{
	// IID: Interface IDentifier
#define _INTERFACE_DEFINE_IID(iid) \
		public: static Integer		__IINTERFACE_IID() { return (iid); }
#define _INTERFACE_IID(interface_) \
		(interface_::__IINTERFACE_IID())
#define _INTERFACE_TEST_IID(interface_, iid) \
		((interface_::__IINTERFACE_IID()) == (iid))

	class IUnknown
	{
	public:
		virtual			~IUnknown() = default;

		virtual bool		QueryInterface(Integer iid, void ** ppvObject)
		{
			return false;
		}
		template <typename T>
		bool			QueryInterface(T ** ppObject)
		{
			return QueryInterface(_INTERFACE_IID(T), ( void ** ) ppObject);
		}
	};
}