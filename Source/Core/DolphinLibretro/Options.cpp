
#include <libretro.h>

#include "DolphinLibretro/Options.h"

namespace Libretro
{
extern retro_environment_t environ_cb;

namespace Options
{
static std::vector<retro_variable> optionsList;
static std::vector<bool*> dirtyPtrList;

template <typename T>
void Option<T>::Register()
{
  if (!m_options.empty())
    return;

  m_options = m_name;
  m_options.push_back(';');
  for (auto& option : m_list)
  {
    if (option.first == m_list.begin()->first)
      m_options += std::string(" ") + option.first;
    else
      m_options += std::string("|") + option.first;
  }
  optionsList.push_back({m_id, m_options.c_str()});
  dirtyPtrList.push_back(&m_dirty);
  Updated();
  m_dirty = true;
}

void SetVariables()
{
  if (optionsList.empty())
    return;
  if (optionsList.back().key)
    optionsList.push_back({});
  environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)optionsList.data());
}

void CheckVariables()
{
  bool updated = false;
  if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && !updated)
    return;

  for (bool* ptr : dirtyPtrList)
    *ptr = true;
}

template <typename T>
Option<T>::Option(const char* id, const char* name,
                  std::initializer_list<std::pair<const char*, T>> list)
    : m_id(id), m_name(name), m_list(list.begin(), list.end())
{
  Register();
}

template <typename T>
Option<T>::Option(const char* id, const char* name, std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, (T)m_list.size()});
  Register();
}
template <>
Option<std::string>::Option(const char* id, const char* name,
                            std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, option});
  Register();
}
template <>
Option<const char*>::Option(const char* id, const char* name,
                            std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, option});
  Register();
}

template <typename T>
Option<T>::Option(const char* id, const char* name, T first,
                  std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, first + (int)m_list.size()});
  Register();
}

template <typename T>
Option<T>::Option(const char* id, const char* name, T first, int count, int step)
    : m_id(id), m_name(name)
{
  for (T i = first; i < first + count; i += step)
    m_list.push_back({std::to_string(i), i});
  Register();
}

template <>
Option<bool>::Option(const char* id, const char* name, bool initial) : m_id(id), m_name(name)
{
  m_list.push_back({initial ? "enabled" : "disabled", initial});
  m_list.push_back({!initial ? "enabled" : "disabled", !initial});
  Register();
}

template <typename T>
bool Option<T>::Updated()
{
  if (m_dirty)
  {
    m_dirty = false;

    retro_variable var{m_id};
    T value = m_list.front().second;

    if (environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
      for (auto option : m_list)
      {
        if (option.first == var.value)
        {
          value = option.second;
          break;
        }
      }
    }

    if (m_value != value)
    {
      m_value = value;
      return true;
    }
  }
  return false;
}

Option<int> efbScale("dolphin_efb_scale", "EFB缩放", 1,
                     {"x1 (640 x 528)", "x2 (1280 x 1056)", "x3 (1920 x 1584)", "x4 (2560 * 2112)",
                      "x5 (3200 x 2640)", "x6 (3840 x 3168)"});
Option<int> irMode("dolphin_ir_mode", "Wiimote IR模式", 1,
                     {"右手柄控制指针（相对模式）", "右手柄控制指针（绝对模式）", "鼠标控制指针"});
Option<int> irCenter("dolphin_ir_center", "Wiimote IR中心",
  { {"50", 50}, {"60", 60}, {"70", 70}, {"80", 80}, {"90", 90}, {"100", 100}, {"0", 0}, {"10", 10}, {"20", 20}, {"30", 30}, {"40", 40} });
Option<int> irWidth("dolphin_ir_width", "Wiimote IR宽度",
  { {"50", 50}, {"60", 60}, {"70", 70}, {"80", 80}, {"90", 90}, {"100", 100}, {"0", 0}, {"10", 10}, {"20", 20}, {"30", 30}, {"40", 40} });
Option<int> irHeight("dolphin_ir_height", "Wiimote IR高度",
  { {"50", 50}, {"60", 60}, {"70", 70}, {"80", 80}, {"90", 90}, {"100", 100}, {"0", 0}, {"10", 10}, {"20", 20}, {"30", 30}, {"40", 40} });
Option<LogTypes::LOG_LEVELS> logLevel("dolphin_log_level", "日志级别",
                                      {{"信息", LogTypes::LINFO},
#if defined(_DEBUG) || defined(DEBUGFAST)
                                       {"调试", LogTypes::LDEBUG},
#endif
                                       {"通知", LogTypes::LNOTICE},
                                       {"错误", LogTypes::LERROR},
                                       {"警告", LogTypes::LWARNING}});
Option<float> cpuClockRate("dolphin_cpu_clock_rate", "CPU时钟",
                           {{"100%", 1.0},
                            {"150%", 1.5},
                            {"200%", 2.0},
                            {"250%", 2.5},
                            {"300%", 3.0},
                            {"5%", 0.05},
                            {"10%", 0.1},
                            {"20%", 0.2},
                            {"30%", 0.3},
                            {"40%", 0.4},
                            {"50%", 0.5},
                            {"60%", 0.6},
                            {"70%", 0.7},
                            {"80%", 0.8},
                            {"90%", 0.9}});
Option<std::string> renderer("dolphin_renderer", "渲染器", {{"硬件","Hardware"}, {"软件","Software"}, {"无","Null"}});
#ifdef ANDROID
Option<bool> fastmem("dolphin_fastmem", "快速内存", false);
#else
Option<bool> fastmem("dolphin_fastmem", "快速内存", true);
#endif
Option<bool> DSPHLE("dolphin_dsp_hle", "高级模拟DSP", true);
Option<bool> DSPEnableJIT("dolphin_dsp_jit", "启用即时重编译DSP", true);
Option<PowerPC::CPUCore> cpu_core("dolphin_cpu_core", "CPU内核",
                                  {
#ifdef _M_X86
                                      {"64位即时重编译器", PowerPC::CPUCore::JIT64},
#elif _M_ARM_64
                                      {"64位ARM即时重编译器", PowerPC::CPUCore::JITARM64},
#endif
                                      {"解释器", PowerPC::CPUCore::Interpreter},
                                      {"缓存解释器", PowerPC::CPUCore::CachedInterpreter}});
Option<DiscIO::Language> Language("dolphin_language", "语言",
                                  {{"英语", DiscIO::Language::English},
                                   {"日语", DiscIO::Language::Japanese},
                                   {"德语", DiscIO::Language::German},
                                   {"法语", DiscIO::Language::French},
                                   {"西班牙语", DiscIO::Language::Spanish},
                                   {"意大利语", DiscIO::Language::Italian},
                                   {"荷兰语", DiscIO::Language::Dutch},
                                   {"简体中文", DiscIO::Language::SimplifiedChinese},
                                   {"繁体中文", DiscIO::Language::TraditionalChinese},
                                   {"韩语", DiscIO::Language::Korean}});
Option<bool> Widescreen("dolphin_widescreen", "宽屏", true);
Option<bool> WidescreenHack("dolphin_widescreen_hack", "宽屏补丁", false);
Option<bool> progressiveScan("dolphin_progressive_scan", "逐行扫描", true);
Option<bool> pal60("dolphin_pal60", "PAL60", true);
Option<u32> sensorBarPosition("dolphin_sensor_bar_position", "感应条位置",
                              {"底部", "顶部"});
Option<bool> bluetoothContinuousScan("dolphin_bt_continuous_scan", "蓝牙扫描", {"关闭", "持续"});
Option<unsigned int> audioMixerRate("dolphin_mixer_rate", "音频混音器采样率",
                                    {{"32000", 32000u}, {"48000", 48000u}});
Option<ShaderCompilationMode> shaderCompilationMode(
    "dolphin_shader_compilation_mode", "着色器编译模式",
    {{"同步", ShaderCompilationMode::Synchronous},
     {"异步跳过渲染", ShaderCompilationMode::AsynchronousSkipRendering},
     {"同步UberShaders", ShaderCompilationMode::SynchronousUberShaders},
     {"异步UberShaders", ShaderCompilationMode::AsynchronousUberShaders}});
Option<int> maxAnisotropy("dolphin_max_anisotropy", "最大各向异性过滤", 0, 17);
Option<bool> efbScaledCopy("dolphin_efb_scaled_copy", "缩放EFB拷贝", true);
Option<bool> efbToTexture("dolphin_efb_to_texture", "在GPU中存储EFB拷贝", true);
Option<bool> efbToVram("dolphin_efb_to_vram", "禁用拷贝EFB到VRAM", false);
Option<bool> bboxEnabled("dolphin_bbox_enabled", "边界框模拟", false);
Option<bool> gpuTextureDecoding("dolphin_gpu_texture_decoding", "GPU纹理解码", false);
Option<bool> waitForShaders("dolphin_wait_for_shaders", "在开始之前等待着色器", false);
Option<bool> forceTextureFiltering("dolphin_force_texture_filtering", "强制纹理过滤", false);
Option<bool> loadCustomTextures("dolphin_load_custom_textures", "载入自定义纹理", false);
Option<bool> cheatsEnabled("dolphin_cheats_enabled", "启用内部金手指", false);
Option<int> textureCacheAccuracy("dolphin_texture_cache_accuracy", "纹理缓存精度",
                                 {{"快速", 128}, {"中等", 512}, {"安全", 0}});
}  // namespace Options
}  // namespace Libretro
