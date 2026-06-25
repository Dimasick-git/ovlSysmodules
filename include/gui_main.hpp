#pragma once
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <sys/stat.h>
#include <dirent.h>
#include <cJSON.h>
#include <algorithm>
#include <tesla.hpp>

// ===========================================================================
// AutoStartToggleListItem
//
// A Switch 2 styled toggle abstraction for the sysmodule list, kept entirely
// outside tesla.hpp/.cpp. It carries two INDEPENDENT, externally-driven flags:
//
//   running   -> drives the sliding pill (KEY_A start/stop). Reuses the exact
//                150 ms smoothstep slide that tsl::elm::ToggleListItem uses.
//   autoStart -> drives the CIRCLE colour only (KEY_Y boot2 flag). On  = the
//                theme's locked trackbar circle colour (trackBarSliderColor),
//                Off = the default Switch 2 toggle circle colour.
//
// Why subclass ListItem and not ToggleListItem: ToggleListItem::onClick hijacks
// a lone KEY_A to flip its own m_state and fire its state-changed listener,
// which would fight the graceful-shutdown click listener set in gui_main.cpp.
// Subclassing ListItem keeps ListItem::onClick, so KEY_A and KEY_Y both reach
// that click listener exactly as before -- input handling is unchanged.
//
// Legacy (non Switch 2) style is untouched: drawValue falls back to the base
// ON/OFF value text (including its existing auto-start glyph prefix).
// ===========================================================================
class AutoStartToggleListItem : public tsl::elm::ListItem {
public:
    // value is the initial ON/OFF text used in legacy style; it also keeps
    // m_value non-empty so the Switch 2 pill renders before the first poll.
    AutoStartToggleListItem(const std::string& text, const std::string& value)
        : tsl::elm::ListItem(text, value) {}

    virtual ~AutoStartToggleListItem() {}

    // Running state -> slide. Snaps on the first call, animates on later changes.
    void setRunning(bool running) {
        if (m_switchInit && running == m_running)
            return;
        m_running = running;
        triggerSwitchAnim(running);
    }

    // Auto-start state -> circle colour. Snaps on the first call, animates
    // (eased colour blend) on later changes.
    void setAutoStart(bool autoStart) {
        if (m_autoStartInit && autoStart == m_autoStart)
            return;
        m_autoStart = autoStart;
        triggerAutoStartAnim(autoStart);
    }

protected:
    // Reserve the same 48+66 px value slot ToggleListItem reserves so the label
    // truncates correctly and the pill's right edge lands where the ON/OFF text
    // right edge would. In legacy style, defer to the base reservation.
    virtual s32 valueReservedWidth(tsl::gfx::Renderer* renderer) override {
        m_widthsForSwitch2Style = ult::useSwitch2Style;
        if (ult::useSwitch2Style)
            return 48 + 66;
        return tsl::elm::ListItem::valueReservedWidth(renderer);
    }

    // Force a width recalc the frame the global style flips (see base hook).
    virtual bool valueReservedWidthChanged() override {
        return m_widthsForSwitch2Style != ult::useSwitch2Style;
    }

    // The pill owns the right side; never extend the Switch 2 scissor rightward.
    virtual bool switch2ExtendsRight() override {
        return false;
    }

    // Switch 2 -> sliding pill; legacy -> base ON/OFF text (unchanged).
    virtual void drawValue(tsl::gfx::Renderer* renderer, s32 yOffset, bool useClickTextColor) override {
        if (ult::useSwitch2Style)
            drawSwitch(renderer, yOffset);
        else
            tsl::elm::ListItem::drawValue(renderer, yOffset, useClickTextColor);
    }

private:
    // ---- slide machinery (ported from tsl::elm::ToggleListItem) -------------
    // Eased slide parameter p in [0,1] (0 = off/left, 1 = on/right), recomputed
    // every frame so an interrupted slide resumes smoothly from where it was.
    float currentSwitchP() const {
        if (!m_switchInit)
            return m_switchTargetP;
        const u64 elapsed = ult::nowNs() - m_switchAnimStartNs;
        if (elapsed >= kSwitchSlideNs)
            return m_switchTargetP;
        const float prog = static_cast<float>(elapsed) / static_cast<float>(kSwitchSlideNs);
        const float e = prog * prog * (3.0f - 2.0f * prog);   // smoothstep (ease in-out)
        return m_switchAnimFromP + (m_switchTargetP - m_switchAnimFromP) * e;
    }

    // Begin a slide toward newState. First call snaps (no animation); later
    // calls slide from the current position, so a rapid re-toggle reverses
    // smoothly mid-travel.
    void triggerSwitchAnim(bool newState) {
        const float target = newState ? 1.0f : 0.0f;
        if (!m_switchInit) {
            m_switchInit = true;
            m_switchTargetP = m_switchAnimFromP = target;
            m_switchAnimStartNs = 0;
            return;
        }
        if (target == m_switchTargetP)
            return;   // already heading there
        m_switchAnimFromP   = currentSwitchP();   // smooth interrupt from current pos
        m_switchTargetP     = target;
        m_switchAnimStartNs = ult::nowNs();
    }

    // ---- colour-blend machinery (mirrors the slide, for the circle colour) --
    // Eased blend parameter q in [0,1] (1 = auto-start on / green, 0 = off /
    // red), recomputed every frame so an interrupted blend resumes smoothly.
    float currentAutoStartQ() const {
        if (!m_autoStartInit)
            return m_autoStartTargetQ;
        const u64 elapsed = ult::nowNs() - m_autoStartAnimStartNs;
        if (elapsed >= kAutoStartBlendNs)
            return m_autoStartTargetQ;
        const float prog = static_cast<float>(elapsed) / static_cast<float>(kAutoStartBlendNs);
        const float e = prog * prog * (3.0f - 2.0f * prog);   // smoothstep (ease in-out)
        return m_autoStartFromQ + (m_autoStartTargetQ - m_autoStartFromQ) * e;
    }

    // Begin a colour blend toward newState. First call snaps (no animation);
    // later calls blend from the current colour, so a rapid re-toggle reverses
    // smoothly mid-fade.
    void triggerAutoStartAnim(bool newState) {
        const float target = newState ? 1.0f : 0.0f;
        if (!m_autoStartInit) {
            m_autoStartInit = true;
            m_autoStartTargetQ = m_autoStartFromQ = target;
            m_autoStartAnimStartNs = 0;
            return;
        }
        if (target == m_autoStartTargetQ)
            return;   // already heading there
        m_autoStartFromQ       = currentAutoStartQ();   // smooth interrupt from current colour
        m_autoStartTargetQ     = target;
        m_autoStartAnimStartNs = ult::nowNs();
    }

    // 48x26 pill with a sliding 10px circle. The track lerps on<->off with the
    // slide p (running). The circle colour is the SOLE auto-start indicator:
    // it eases between light green (auto-start on) and light red (off) via the
    // blend q, so toggling auto-start fades rather than snaps.
    void drawSwitch(tsl::gfx::Renderer* renderer, s32 yOffset) {
        (void)yOffset;
        static constexpr s32 kTrackW  = 48;
        static constexpr s32 kTrackH  = 26;
        static constexpr s32 kRadius  = kTrackH / 2;     // 13 (uniform pill radius)
        static constexpr s32 kGap     = 3;               // circle inset from track edge
        static constexpr s32 kCircleR = kRadius - kGap;  // 10

        if (!m_switchInit) {  // lazy snap if setRunning was never called yet
            m_switchInit = true;
            m_switchTargetP = m_switchAnimFromP = (m_running ? 1.0f : 0.0f);
            m_switchAnimStartNs = 0;
        }
        if (!m_autoStartInit) {  // lazy snap if setAutoStart was never called yet
            m_autoStartInit = true;
            m_autoStartTargetQ = m_autoStartFromQ = (m_autoStart ? 1.0f : 0.0f);
            m_autoStartAnimStartNs = 0;
        }

        // Right edge aligns with where the ON/OFF text right edge would land
        // (m_maxWidth already reserved kTrackW); vertically centred in the row.
        const s32 trackX = this->getX() + m_maxWidth + 47;
        const s32 trackY = this->getY() + (this->getHeight() - kTrackH + 2) / 2;

        const float p = currentSwitchP();
        const float q = currentAutoStartQ();

        const tsl::Color trackColor =
            tsl::lerpColor(tsl::s2ToggleOnColor, tsl::s2ToggleOffColor, p);  // p=1 on, p=0 off
        // Circle colour eases between light green (on) and light red (off).
        const tsl::Color circleColor =
            tsl::lerpColor(kAutoStartOnColor, kAutoStartOffColor, q);  // q=1 green, q=0 red

        renderer->drawUniformRoundedRect(trackX, trackY, kTrackW, kTrackH, a(trackColor));

        const s32 circleCX = trackX + kRadius
            + static_cast<s32>(p * static_cast<float>(kTrackW - 2 * kRadius) + 0.5f);
        const s32 circleCY = trackY + kRadius;
        renderer->drawCircle(circleCX, circleCY, static_cast<u16>(kCircleR), true, a(circleColor));
    }

    static constexpr u64 kSwitchSlideNs = 150000000ULL;  // 150 ms quick slide
    bool  m_switchInit        = false;
    float m_switchAnimFromP   = 0.0f;
    float m_switchTargetP     = 0.0f;
    u64   m_switchAnimStartNs = 0;

    // Circle colour blend (auto-start). Mirrors the slide constants/fields.
    // Colours are RGBA4444 (each channel 0-15); a() applies the global fade.
    static constexpr u64 kAutoStartBlendNs = 100000000ULL;  // 100 ms colour fade
    static constexpr tsl::Color kAutoStartOnColor  = tsl::Color(15, 15, 15, 15);  // light green
    static constexpr tsl::Color kAutoStartOffColor = 0xF66F;  // light red
    bool  m_autoStartInit        = false;
    float m_autoStartFromQ       = 0.0f;
    float m_autoStartTargetQ     = 0.0f;
    u64   m_autoStartAnimStartNs = 0;

    // Style under which m_maxWidth was last computed; see valueReservedWidthChanged().
    bool m_widthsForSwitch2Style = ult::useSwitch2Style;

    bool m_running   = false;
    bool m_autoStart = false;
};

struct SystemModule {
    AutoStartToggleListItem *listItem;
    u64 programId;
    bool needReboot;
    char flagPath[FS_MAX_PATH];    // Cached flag file path
    char folderPath[FS_MAX_PATH];  // Cached flags folder path
    std::string displayName;       // Store original name + version
    std::string titleIdStr;        // Store formatted title ID

    // Optional graceful-shutdown contract (read from toolbox.json).
    // If hasGracefulShutdown is true, we attempt an IPC-driven cleanup
    // before pmshellTerminateProgram; if absent or it fails/times out,
    // we fall back to instant force-kill (the original behavior).
    // This makes the graceful path completely opt-in per sysmodule —
    // ovlSysmodules never hardcodes knowledge of any specific module.
    bool hasGracefulShutdown = false;
    char gracefulShutdownService[16] = {};  // libnx service name limit is 8 chars + null
    u32  gracefulShutdownCmd = 0;
    u32  gracefulShutdownTimeoutMs = 1000;
};

class GuiMain : public tsl::Gui {
  private:
    std::vector<SystemModule> m_sysmoduleListItems;
    bool m_scanned = false;
    bool m_isActive = true;
    bool m_showTitleIds = false;  // Toggle state

    // Off-thread IPC worker (see spawnIpcThread()/update()).
    //
    // Blocking IPC calls (pmshellLaunchProgram, pmshellTerminateProgram,
    // tryGracefulShutdown) run on m_ipcThread so the render thread never
    // stalls mid-animation. update() joins the thread once m_ipcDone fires,
    // which is always cheap because the thread has already finished by then.
    //
    // Only one IPC operation runs at a time. If the user clicks again while
    // a thread is still live, spawnIpcThread() waits for the previous one
    // to finish (it's already near completion at that point) before starting
    // the new one — correct serialisation at negligible cost.
    std::thread      m_ipcThread;
    std::atomic<bool> m_ipcDone{true};   // true = no thread in flight / thread finished

  public:
    GuiMain();
    ~GuiMain();
    
    virtual tsl::elm::Element *createUI();
    virtual void update() override;
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override;
    
  private:
    void updateStatus(const SystemModule &module);
    bool hasFlag(const SystemModule &module);
    bool isRunning(const SystemModule &module);
    void toggleTitleIdDisplay();
    void spawnIpcThread(std::function<void()> action);
};