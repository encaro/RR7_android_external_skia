/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_Base.h"
#include "SkCommonFlagsConfig.h"
#include "SkImageInfo.h"

#include <stdlib.h>

#if SK_SUPPORT_GPU
using sk_gpu_test::GrContextFactory;
#endif

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_IOS)
#    define DEFAULT_GPU_CONFIG "gles"
#else
#    define DEFAULT_GPU_CONFIG "gl"
#endif

static const char defaultConfigs[] =
    "8888 " DEFAULT_GPU_CONFIG " nonrendering "
#if defined(SK_BUILD_FOR_WIN)
    " angle_d3d11_es2"
#endif
    ;

#undef DEFAULT_GPU_CONFIG

static const struct {
    const char* predefinedConfig;
    const char* backend;
    const char* options;
} gPredefinedConfigs[] ={
#if SK_SUPPORT_GPU
    { "gl",                    "gpu", "api=gl" },
    { "gles",                  "gpu", "api=gles" },
    { "glmsaa4",               "gpu", "api=gl,samples=4" },
    { "glmsaa16",              "gpu", "api=gl,samples=16" },
    { "glesmsaa4",             "gpu", "api=gles,samples=4" },
    { "glnvpr4",               "gpu", "api=gl,nvpr=true,samples=4" },
    { "glnvpr16",              "gpu", "api=gl,nvpr=true,samples=16" },
    { "glnvprdit4",            "gpu", "api=gl,nvpr=true,samples=4,dit=true" },
    { "glnvprdit16",           "gpu", "api=gl,nvpr=true,samples=16,dit=true" },
    { "glesnvpr4",             "gpu", "api=gles,nvpr=true,samples=4" },
    { "glesnvprdit4",          "gpu", "api=gles,nvpr=true,samples=4,dit=true" },
    { "glinst",                "gpu", "api=gl,inst=true" },
    { "glinst4",               "gpu", "api=gl,inst=true,samples=4" },
    { "glinstdit4",            "gpu", "api=gl,inst=true,samples=4,dit=true" },
    { "glinst16",              "gpu", "api=gl,inst=true,samples=16" },
    { "glinstdit16",           "gpu", "api=gl,inst=true,samples=16,dit=true" },
    { "glesinst",              "gpu", "api=gles,inst=true" },
    { "glesinst4",             "gpu", "api=gles,inst=true,samples=4" },
    { "glesinstdit4",          "gpu", "api=gles,inst=true,samples=4,dit=true" },
    { "glf16",                 "gpu", "api=gl,color=f16" },
    { "glsrgb",                "gpu", "api=gl,color=srgb" },
    { "glsrgbnl",              "gpu", "api=gl,color=srgbnl" },
    { "glesf16",               "gpu", "api=gles,color=f16" },
    { "glessrgb",              "gpu", "api=gles,color=srgb" },
    { "glessrgbnl",            "gpu", "api=gles,color=srgbnl" },
    { "glsrgb",                "gpu", "api=gl,color=srgb" },
    { "glwide",                "gpu", "api=gl,color=f16_wide" },
    { "glnarrow",              "gpu", "api=gl,color=f16_narrow" },
    { "glessrgb",              "gpu", "api=gles,color=srgb" },
    { "gleswide",              "gpu", "api=gles,color=f16_wide" },
    { "glesnarrow",            "gpu", "api=gles,color=f16_narrow" },
    { "gldft",                 "gpu", "api=gl,dit=true" },
    { "glesdft",               "gpu", "api=gles,dit=true" },
    { "debuggl",               "gpu", "api=debuggl" },
    { "nullgl",                "gpu", "api=nullgl" },
    { "angle_d3d11_es2",       "gpu", "api=angle_d3d11_es2" },
    { "angle_d3d11_es3",       "gpu", "api=angle_d3d11_es3" },
    { "angle_d3d9_es2",        "gpu", "api=angle_d3d9_es2" },
    { "angle_d3d11_es2_msaa4", "gpu", "api=angle_d3d11_es2,samples=4" },
    { "angle_gl_es2",          "gpu", "api=angle_gl_es2" },
    { "angle_gl_es3",          "gpu", "api=angle_gl_es3" },
    { "commandbuffer",         "gpu", "api=commandbuffer" }
#if SK_MESA
    ,{ "mesa",                 "gpu", "api=mesa" }
#endif
#ifdef SK_VULKAN
    ,{ "vk",                   "gpu", "api=vulkan" }
    ,{ "vksrgb",               "gpu", "api=vulkan,color=srgb" }
    ,{ "vkwide",               "gpu", "api=vulkan,color=f16_wide" }
    ,{ "vkmsaa4",              "gpu", "api=vulkan,samples=4" }
    ,{ "vkmsaa16",             "gpu", "api=vulkan,samples=16" }
#endif

#else
{ "", "", "" }
#endif
};

static const char configHelp[] =
    "Options: 565 8888 srgb f16 nonrendering null pdf pdfa skp pipe svg xps";

static const char* config_help_fn() {
    static SkString helpString;
    helpString.set(configHelp);
    for (const auto& config : gPredefinedConfigs) {
        helpString.appendf(" %s", config.predefinedConfig);
    }
    helpString.append(" or use extended form 'backend[option=value,...]'.\n");
    return helpString.c_str();
}

static const char configExtendedHelp[] =
    "Extended form: 'backend(option=value,...)'\n\n"
    "Possible backends and options:\n"
#if SK_SUPPORT_GPU
    "\n"
    "gpu[api=string,color=string,dit=bool,nvpr=bool,inst=bool,samples=int]\n"
    "\tapi\ttype: string\trequired\n"
    "\t    Select graphics API to use with gpu backend.\n"
    "\t    Options:\n"
    "\t\tnative\t\t\tUse platform default OpenGL or OpenGL ES backend.\n"
    "\t\tgl    \t\t\tUse OpenGL.\n"
    "\t\tgles  \t\t\tUse OpenGL ES.\n"
    "\t\tdebuggl \t\t\tUse debug OpenGL.\n"
    "\t\tnullgl \t\t\tUse null OpenGL.\n"
    "\t\tangle_d3d9_es2\t\t\tUse OpenGL ES2 on the ANGLE Direct3D9 backend.\n"
    "\t\tangle_d3d11_es2\t\t\tUse OpenGL ES2 on the ANGLE Direct3D11 backend.\n"
    "\t\tangle_d3d11_es3\t\t\tUse OpenGL ES3 on the ANGLE Direct3D11 backend.\n"
    "\t\tangle_gl_es2\t\t\tUse OpenGL ES2 on the ANGLE OpenGL backend.\n"
    "\t\tangle_gl_es3\t\t\tUse OpenGL ES3 on the ANGLE OpenGL backend.\n"
    "\t\tcommandbuffer\t\tUse command buffer.\n"
#if SK_MESA
    "\t\tmesa\t\t\tUse MESA.\n"
#endif
#ifdef SK_VULKAN
    "\t\tvulkan\t\t\tUse Vulkan.\n"
#endif
    "\tcolor\ttype: string\tdefault: 8888.\n"
    "\t    Select framebuffer color format.\n"
    "\t    Options:\n"
    "\t\t8888\t\t\tLinear 8888.\n"
    "\t\tf16{_gamut}\t\tLinear 16-bit floating point.\n"
    "\t\tsrgb{_gamut}\t\tsRGB 8888.\n"
    "\t  gamut\ttype: string\tdefault: srgb.\n"
    "\t    Select color gamut for f16 or sRGB format buffers.\n"
    "\t    Options:\n"
    "\t\tsrgb\t\t\tsRGB gamut.\n"
    "\t\twide\t\t\tWide Gamut RGB.\n"
    "\tdit\ttype: bool\tdefault: false.\n"
    "\t    Use device independent text.\n"
    "\tnvpr\ttype: bool\tdefault: false.\n"
    "\t    Use NV_path_rendering OpenGL and OpenGL ES extension.\n"
    "\tsamples\ttype: int\tdefault: 0.\n"
    "\t    Use multisampling with N samples.\n"
    "\n"
    "Predefined configs:\n\n"
    // Help text for pre-defined configs is auto-generated from gPredefinedConfigs
#endif
    ;

static const char* config_extended_help_fn() {
    static SkString helpString;
    helpString.set(configExtendedHelp);
    for (const auto& config : gPredefinedConfigs) {
        helpString.appendf("\t%-10s\t= gpu(%s)\n", config.predefinedConfig, config.options);
    }
    return helpString.c_str();
}

DEFINE_extended_string(config, defaultConfigs, config_help_fn(), config_extended_help_fn());

SkCommandLineConfig::SkCommandLineConfig(const SkString& tag, const SkString& backend,
                                         const SkTArray<SkString>& viaParts)
        : fTag(tag)
        , fBackend(backend)
        , fViaParts(viaParts) {
}
SkCommandLineConfig::~SkCommandLineConfig() {
}

#if SK_SUPPORT_GPU
SkCommandLineConfigGpu::SkCommandLineConfigGpu(
    const SkString& tag, const SkTArray<SkString>& viaParts, ContextType contextType, bool useNVPR,
    bool useInstanced, bool useDIText, int samples, SkColorType colorType,
    sk_sp<SkColorSpace> colorSpace)
        : SkCommandLineConfig(tag, SkString("gpu"), viaParts)
        , fContextType(contextType)
        , fContextOverrides(ContextOverrides::kNone)
        , fUseDIText(useDIText)
        , fSamples(samples)
        , fColorType(colorType)
        , fColorSpace(std::move(colorSpace)) {
    if (useNVPR) {
        fContextOverrides |= ContextOverrides::kRequireNVPRSupport;
    } else if (!useInstanced) {
        // We don't disable NVPR for instanced configs. Otherwise the caps wouldn't use mixed
        // samples and we couldn't test the mixed samples backend for simple shapes.
        fContextOverrides |= ContextOverrides::kDisableNVPR;
    }
    if (useInstanced) {
        fContextOverrides |= ContextOverrides::kUseInstanced;
    }
    // Subtle logic: If the config has a color space attached, we're going to be rendering to sRGB,
    // so we need that capability. In addition, to get the widest test coverage, we DO NOT require
    // that we can disable sRGB decode. (That's for rendering sRGB sources to legacy surfaces).
    //
    // If the config doesn't have a color space attached, we're going to be rendering in legacy
    // mode. In that case, we don't require sRGB capability and we defer to the client to decide on
    // sRGB decode control.
    if (fColorSpace) {
        fContextOverrides |= ContextOverrides::kRequireSRGBSupport;
        fContextOverrides |= ContextOverrides::kAllowSRGBWithoutDecodeControl;
    }
}
static bool parse_option_int(const SkString& value, int* outInt) {
    if (value.isEmpty()) {
        return false;
    }
    char* endptr = nullptr;
    long intValue = strtol(value.c_str(), &endptr, 10);
    if (*endptr != '\0') {
        return false;
    }
    *outInt = static_cast<int>(intValue);
    return true;
}
static bool parse_option_bool(const SkString& value, bool* outBool) {
    if (value.equals("true")) {
        *outBool = true;
        return true;
    }
    if (value.equals("false")) {
        *outBool = false;
        return true;
    }
    return false;
}
static bool parse_option_gpu_api(const SkString& value,
                                 SkCommandLineConfigGpu::ContextType* outContextType) {
    if (value.equals("gl")) {
        *outContextType = GrContextFactory::kGL_ContextType;
        return true;
    }
    if (value.equals("gles")) {
        *outContextType = GrContextFactory::kGLES_ContextType;
        return true;
    }
    if (value.equals("debuggl")) {
        *outContextType = GrContextFactory::kDebugGL_ContextType;
        return true;
    }
    if (value.equals("nullgl")) {
        *outContextType = GrContextFactory::kNullGL_ContextType;
        return true;
    }
    if (value.equals("angle_d3d9_es2")) {
        *outContextType = GrContextFactory::kANGLE_D3D9_ES2_ContextType;
        return true;
    }
    if (value.equals("angle_d3d11_es2")) {
        *outContextType = GrContextFactory::kANGLE_D3D11_ES2_ContextType;
        return true;
    }
    if (value.equals("angle_d3d11_es3")) {
        *outContextType = GrContextFactory::kANGLE_D3D11_ES3_ContextType;
        return true;
    }
    if (value.equals("angle_gl_es2")) {
        *outContextType = GrContextFactory::kANGLE_GL_ES2_ContextType;
        return true;
    }
    if (value.equals("angle_gl_es3")) {
        *outContextType = GrContextFactory::kANGLE_GL_ES3_ContextType;
        return true;
    }
    if (value.equals("commandbuffer")) {
        *outContextType = GrContextFactory::kCommandBuffer_ContextType;
        return true;
    }
#if SK_MESA
    if (value.equals("mesa")) {
        *outContextType = GrContextFactory::kMESA_ContextType;
        return true;
    }
#endif
#ifdef SK_VULKAN
    if (value.equals("vulkan")) {
        *outContextType = GrContextFactory::kVulkan_ContextType;
        return true;
    }
#endif
    return false;
}
static bool parse_option_gpu_color(const SkString& value,
                                   SkColorType* outColorType,
                                   sk_sp<SkColorSpace>* outColorSpace) {
    if (value.equals("8888")) {
        *outColorType = kRGBA_8888_SkColorType;
        *outColorSpace = nullptr;
        return true;
    }

    SkTArray<SkString> commands;
    SkStrSplit(value.c_str(), "_", &commands);
    if (commands.count() < 1 || commands.count() > 2) {
        return false;
    }

    const bool linearGamma = commands[0].equals("f16");
    const bool nonLinearBlending = commands[0].equals("srgbnl");
    SkColorSpace::Gamut gamut = SkColorSpace::kSRGB_Gamut;
    SkColorSpace::RenderTargetGamma gamma = linearGamma ? SkColorSpace::kLinear_RenderTargetGamma
                                                        : SkColorSpace::kSRGB_RenderTargetGamma;
    SkColorSpace::ColorSpaceFlags flags =
            nonLinearBlending ? SkColorSpace::kNonLinearBlending_ColorSpaceFlag
                              : (SkColorSpace::ColorSpaceFlags) 0;
    *outColorSpace = SkColorSpace::MakeRGB(gamma, gamut, flags);

    if (commands.count() == 2) {
        if (commands[1].equals("srgb")) {
            // sRGB gamut (which is our default)
        } else if (commands[1].equals("wide")) {
            // WideGamut RGB
            const float gWideGamutRGB_toXYZD50[]{
                0.7161046f, 0.1009296f, 0.1471858f,  // -> X
                0.2581874f, 0.7249378f, 0.0168748f,  // -> Y
                0.0000000f, 0.0517813f, 0.7734287f,  // -> Z
            };
            SkMatrix44 wideGamutRGBMatrix(SkMatrix44::kUninitialized_Constructor);
            wideGamutRGBMatrix.set3x3RowMajorf(gWideGamutRGB_toXYZD50);
            *outColorSpace = SkColorSpace::MakeRGB(gamma, wideGamutRGBMatrix, flags);
        } else if (commands[1].equals("narrow")) {
            // NarrowGamut RGB (an artifically smaller than sRGB gamut)
            SkColorSpacePrimaries primaries ={
                0.54f, 0.33f,     // Rx, Ry
                0.33f, 0.50f,     // Gx, Gy
                0.25f, 0.20f,     // Bx, By
                0.3127f, 0.3290f, // Wx, Wy
            };
            SkMatrix44 narrowGamutRGBMatrix(SkMatrix44::kUninitialized_Constructor);
            primaries.toXYZD50(&narrowGamutRGBMatrix);
            *outColorSpace = SkColorSpace::MakeRGB(gamma, narrowGamutRGBMatrix, flags);
        } else {
            // Unknown color gamut
            return false;
        }
    }

    // Now pick a color type
    if (commands[0].equals("f16")) {
        *outColorType = kRGBA_F16_SkColorType;
        return true;
    }
    if (commands[0].equals("srgb") || commands[0].equals("srgbnl")) {
        *outColorType = kRGBA_8888_SkColorType;
        return true;
    }
    return false;
}

SkCommandLineConfigGpu* parse_command_line_config_gpu(const SkString& tag,
                                                      const SkTArray<SkString>& vias,
                                                      const SkString& options) {
    // Defaults for GPU backend.
    bool seenAPI = false;
    SkCommandLineConfigGpu::ContextType contextType = GrContextFactory::kGL_ContextType;
    bool seenUseNVPR = false;
    bool useNVPR = false;
    bool seenUseInstanced = false;
    bool useInstanced = false;
    bool seenUseDIText =false;
    bool useDIText = false;
    bool seenSamples = false;
    int samples = 0;
    bool seenColor = false;
    SkColorType colorType = kRGBA_8888_SkColorType;
    sk_sp<SkColorSpace> colorSpace = nullptr;

    SkTArray<SkString> optionParts;
    SkStrSplit(options.c_str(), ",", kStrict_SkStrSplitMode, &optionParts);
    for (int i = 0; i < optionParts.count(); ++i) {
        SkTArray<SkString> keyValueParts;
        SkStrSplit(optionParts[i].c_str(), "=", kStrict_SkStrSplitMode, &keyValueParts);
        if (keyValueParts.count() != 2) {
            return nullptr;
        }
        const SkString& key = keyValueParts[0];
        const SkString& value = keyValueParts[1];
        bool valueOk = false;
        if (key.equals("api") && !seenAPI) {
            valueOk = parse_option_gpu_api(value, &contextType);
            seenAPI = true;
        } else if (key.equals("nvpr") && !seenUseNVPR) {
            valueOk = parse_option_bool(value, &useNVPR);
            seenUseNVPR = true;
        } else if (key.equals("inst") && !seenUseInstanced) {
            valueOk = parse_option_bool(value, &useInstanced);
            seenUseInstanced = true;
        } else if (key.equals("dit") && !seenUseDIText) {
            valueOk = parse_option_bool(value, &useDIText);
            seenUseDIText = true;
        } else if (key.equals("samples") && !seenSamples) {
            valueOk = parse_option_int(value, &samples);
            seenSamples = true;
        } else if (key.equals("color") && !seenColor) {
            valueOk = parse_option_gpu_color(value, &colorType, &colorSpace);
            seenColor = true;
        }
        if (!valueOk) {
            return nullptr;
        }
    }
    if (!seenAPI) {
        return nullptr;
    }
    return new SkCommandLineConfigGpu(tag, vias, contextType, useNVPR, useInstanced, useDIText,
                                      samples, colorType, colorSpace);
}
#endif

void ParseConfigs(const SkCommandLineFlags::StringArray& configs,
                  SkCommandLineConfigArray* outResult) {
    outResult->reset();
    for (int i = 0; i < configs.count(); ++i) {
        SkString extendedBackend;
        SkString extendedOptions;
        SkString simpleBackend;
        SkTArray<SkString> vias;

        SkString tag(configs[i]);
        SkTArray<SkString> parts;
        SkStrSplit(tag.c_str(), "[", kStrict_SkStrSplitMode, &parts);
        if (parts.count() == 2) {
            SkTArray<SkString> parts2;
            SkStrSplit(parts[1].c_str(), "]", kStrict_SkStrSplitMode, &parts2);
            if (parts2.count() == 2 && parts2[1].isEmpty()) {
                SkStrSplit(parts[0].c_str(), "-", kStrict_SkStrSplitMode, &vias);
                if (vias.count()) {
                    extendedBackend = vias[vias.count() - 1];
                    vias.pop_back();
                } else {
                    extendedBackend = parts[0];
                }
                extendedOptions = parts2[0];
                simpleBackend.printf("%s[%s]", extendedBackend.c_str(), extendedOptions.c_str());
            }
        }

        if (extendedBackend.isEmpty()) {
            simpleBackend = tag;
            SkStrSplit(tag.c_str(), "-", kStrict_SkStrSplitMode, &vias);
            if (vias.count()) {
                simpleBackend = vias[vias.count() - 1];
                vias.pop_back();
            }
            for (auto& predefinedConfig : gPredefinedConfigs) {
                if (simpleBackend.equals(predefinedConfig.predefinedConfig)) {
                    extendedBackend = predefinedConfig.backend;
                    extendedOptions = predefinedConfig.options;
                    break;
                }
            }
        }
        SkCommandLineConfig* parsedConfig = nullptr;
#if SK_SUPPORT_GPU
        if (extendedBackend.equals("gpu")) {
            parsedConfig = parse_command_line_config_gpu(tag, vias, extendedOptions);
        }
#endif
        if (!parsedConfig) {
            parsedConfig = new SkCommandLineConfig(tag, simpleBackend, vias);
        }
        outResult->emplace_back(parsedConfig);
    }
}
