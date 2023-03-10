// I'll need access to Usd as well
using namespace pxr;

static std::string hello(const ON_wString& name)
{
  ON_wString greeting;
  greeting.Format(L"Hello %s", name);
  return greeting;
}
