//==========================================================================;
// wxdebug.h  -  Minimal debug stub for the ASIO SDK combase helpers.
// Only combase.cpp uses these; JUCE's ASIO host does not compile combase.cpp.
//==========================================================================;

#ifndef __WXDEBUG__
#define __WXDEBUG__

#ifdef DEBUG
#  define DbgLog(x)
#  define DbgRegisterObjectCreation(name)   ((DWORD)0)
#  define DbgRegisterObjectDestruction(c)
#  define CheckPointer(p,r)  if(!(p)) return (r)
#  define ValidateReadWritePtr(p,n)
#  define ASSERT(x)
#else
#  define DbgLog(x)
#  define DbgRegisterObjectCreation(name)   ((DWORD)0)
#  define DbgRegisterObjectDestruction(c)
#  define CheckPointer(p,r)  if(!(p)) return (r)
#  define ValidateReadWritePtr(p,n)
#  define ASSERT(x)
#endif

#endif // __WXDEBUG__
