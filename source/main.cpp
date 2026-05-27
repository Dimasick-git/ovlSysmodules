#define TESLA_INIT_IMPL
#include <exception_wrap.hpp>
#include "gui_main.hpp"
#include "ovls_lang.hpp"

class OverlaySysmodules : public tsl::Overlay {
  public:
    OverlaySysmodules() {}
    ~OverlaySysmodules() {}

    void initServices() override {
        pmshellInitialize();

        // Resolve system language -> 2-letter code, then load the
        // matching lang/<code>.json. set:sys is part of the standard
        // Tesla init bundle so it's safe to call here.
        u64 langCode = 0;
        SetLanguage lang = SetLanguage_ENUS;
        if (R_SUCCEEDED(setInitialize())) {
            if (R_SUCCEEDED(setGetSystemLanguage(&langCode))) {
                setMakeLanguage(langCode, &lang);
            }
            setExit();
        }
        switch (lang) {
            case SetLanguage_RU: ovls::loadLanguage("ru"); break;
            default:                  ovls::loadLanguage("en"); break;
        }
    }

    void exitServices() override {
        pmshellExit();
    }

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return std::make_unique<GuiMain>();
    }
};

int main(int argc, char** argv) {
    return tsl::loop<OverlaySysmodules>(argc, argv);
}