// Debugio.h: Defines a set of macros and definitions to allow for automatically diabled debugging output.
// Note: DO NOT USE std::endl in any debugging output statement! It will work in debugging, but not in release!

#ifndef DEBUGIO_H
#define DEBUGIO_H
#pragma once
#include <iostream>
#pragma warning(push)
#pragma warning(disable:4005)
#ifdef _DEBUG
#ifndef NDEBUG
#define DEBUGGING
#endif 
#endif

#ifdef DEBUGGING
std::ostream& debug_ostream (std::cerr); // Cerr used because it is unbuffered, and so gives up-to-date information.
std::ostream& debug_errstream (std::cerr);
std::ostream& debug (debug_ostream);
template <class T> void debug_print(const T& x)
{
	std::cerr << x;
}

template <class T> void debug_error(const T& x)
{
	std::cerr<<x;
	throw "Debug_error was called.";
}

template <class T, class C> void debug_error_err(const T& x, const C& err)
{
	std::err<<x;
	throw err;
}
#else
namespace hidden // To hide functionality from programmers
{
	namespace debugio
	{
		namespace IfYouUseThisInDebuggingYouWillCertainlyDie // Not defined outside of release. Seriously, let this header take care of stuff in this namespace. Please.
		{
			class OBlackHole
			{
			public:
				// All default functions. I really don't care what you do with these, this class has no data members anyway.
				virtual ~OBlackHole() throw () {} // I do intend to inherit from this, so...

				template <class T> const OBlackHole& operator << (const T&) const throw () // Absorbs its argument and does nothing at all, except allowing call chaining.
				{
					return *this;
				}
			} OutputAbsorber;

			class IBlackHole
			{
			public:
				// All default functions. I really don't care what you do with these, this class has no data members anyway.
				virtual ~IBlackHole() throw () {} // I also intend to inherit from this, so...

				template <class T> const IBlackHole& operator >> (T&) const throw () // Does not modify its argument, but doesn't allow calling with a const parameter, for semantics sake. Allows call chaining.
				{
					return *this;
				}
			} InputAbsorber;

			class IOBlackHole : virtual public OBlackHole, virtual public IBlackHole // Insert both functionalities
			{
			public:
				virtual ~IOBlackHole() throw () {} // For sanity's sake.
			} InputOutputAborber;
		}
	}
}

hidden::debugio::IfYouUseThisInDebuggingYouWillCertainlyDie::OBlackHole& debug_ostream;
hidden::debugio::IfYouUseThisInDebuggingYouWillCertainlyDie::OBlackHole& debug_errstream;
hidden::debugio::IfYouUseThisInDebuggingYouWillCertainlyDie::OBlackHole& debug;
template <class T> void debug_print(const T&) throw () {}
template <class T> void debug_error(const T&) throw () {}
template <class T, class C> void debug_error_err(const T&, const C&) throw () {}
#endif
#pragma warning(pop)
#endif