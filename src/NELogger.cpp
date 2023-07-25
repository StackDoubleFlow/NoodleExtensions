
#include "NELogger.h"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"

#include <unwind.h>
#include <cxxabi.h>

namespace {

struct BacktraceState {
  void** current;
  void** end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg) {
  BacktraceState* state = static_cast<BacktraceState*>(arg);
  uintptr_t pc = _Unwind_GetIP(context);
  if (pc) {
    if (state->current == state->end) {
      return _URC_END_OF_STACK;
    } else {
      *state->current++ = reinterpret_cast<void*>(pc);
    }
  }
  return _URC_NO_REASON;
}

size_t captureBacktrace(void** buffer, size_t max) {
  BacktraceState state = { buffer, buffer + max };
  _Unwind_Backtrace(unwindCallback, &state);

  return state.current - buffer;
}

} // end namespace

void PrintBacktrace(size_t maxLines) {
  size_t lines = maxLines + 1;
  void* buffer[lines];
  captureBacktrace(buffer, lines);
  NELogger::GetLogger().debug("Printing backtrace with %zu max lines:", maxLines);
  NELogger::GetLogger().debug("*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***");
  NELogger::GetLogger().debug("pid: %i, tid: %i", getpid(), gettid());
  for (int i = 0; i < maxLines; i++) {
    Dl_info info;
    if (dladdr(buffer[i + 1], &info)) {
      // For some reason it's always one instruction ahead
      long addr = (char*)buffer[i + 1] - (char*)info.dli_fbase - 4;
      if (info.dli_sname) {
        int status;
        char const* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        if (status) {
          demangled = info.dli_sname;
        }
        NELogger::GetLogger().debug("        #%02i  pc %016lx  %s (%s)", i, addr, info.dli_fname, demangled);
      } else {
        NELogger::GetLogger().debug("        #%02i  pc %016lx  %s", i, addr, info.dli_fname);
      }
    }
  }
  NELogger::GetLogger().debug("Backtrace end.");
}

void PrintJSONValue(rapidjson::Value const& json) {
  using namespace rapidjson;

  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  json.Accept(writer);
  auto str = sb.GetString();
  NELogger::GetLogger().info("%s", str);
}