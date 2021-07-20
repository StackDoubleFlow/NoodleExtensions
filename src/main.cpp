// It doesn't like it if i include this after
#include "NEConfig.h"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "custom-types/shared/logging.hpp"
#include "modloader/shared/modloader.hpp"

#include "NEHooks.h"
#include "NELogger.h"

#include <iostream>
#include <string>
#include <unwind.h>

#include "GlobalNamespace/BeatmapData.hpp"
#include "System/Exception.hpp"

void PrintJSONValue(const rapidjson::Value &json) {
    using namespace rapidjson;

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    json.Accept(writer);
    auto str = sb.GetString();
    NELogger::GetLogger().info("%s", str);
}

namespace {

struct BacktraceState {
    void **current;
    void **end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context *context,
                                          void *arg) {
    BacktraceState *state = static_cast<BacktraceState *>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = reinterpret_cast<void *>(pc);
        }
    }
    return _URC_NO_REASON;
}

size_t captureBacktrace(void **buffer, size_t max) {
    BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(unwindCallback, &state);

    return state.current - buffer;
}

} // end namespace

void PrintBacktrace(size_t maxLines) { 
    size_t lines = maxLines + 1;
    void *buffer[lines];
    captureBacktrace(buffer, lines);
    NELogger::GetLogger().debug("Printing backtrace with %zu max lines:", maxLines);
    NELogger::GetLogger().debug("*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***");
    NELogger::GetLogger().debug("pid: %i, tid: %i", getpid(), gettid());
    for (int i = 1; i < lines; i++) {
        Dl_info info;
        if (dladdr(buffer[i], &info)) {
            long addr = (char *) buffer[i] - (char *) info.dli_fbase;
            if (info.dli_sname) {
                int status;
                const char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
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

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = VERSION;
    NELogger::modInfo = info;
    getNEConfig().Init(info);
}

void InstallNEConfigHooks(Logger &logger);

extern "C" void load() {
    NELogger::GetLogger().info("Installing NoodleExtensions Hooks!");
    // This prevents any and all Utils logging
    // Logger::get().options.silent = false;

    // Install hooks

    InstallAndRegisterAll();

    // what the fuck
    InstallNEConfigHooks(NELogger::GetLogger());

    NELogger::GetLogger().info("Installed NoodleExtensions Hooks!");
}
