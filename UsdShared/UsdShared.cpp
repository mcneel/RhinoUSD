#include "stdafx.h"

// I'll need access to Usd as well
using namespace pxr;

static ON_wString hello(const ON_wString& name)
{
  ON_wString greeting;
  greeting.Format(L"Hello %s", name.Array());
  return greeting;
}
