//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023 - present Mikael Sundell.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

// imath
#include <Imath/ImathMatrix.h>
#include <Imath/ImathVec.h>

// openimageio
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/typedesc.h>
#include <OpenImageIO/argparse.h>
#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/sysutil.h>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

using namespace OIIO;

// prints
template <typename T>
static void
print_info(std::string param, const T& value = T()) {
    std::cout << "info: " << param << value << std::endl;
}

static void
print_info(std::string param) {
    print_info<std::string>(param);
}

template <typename T>
static void
print_warning(std::string param, const T& value = T()) {
    std::cout << "warning: " << param << value << std::endl;
}

static void
print_warning(std::string param) {
    print_warning<std::string>(param);
}

template <typename T>
static void
print_error(std::string param, const T& value = T()) {
    std::cerr << "error: " << param << value << std::endl;
}

static void
print_error(std::string param) {
    print_error<std::string>(param);
}

// title tool
struct TitleTool
{
    bool help = false;
    bool verbose = false;
    std::string title;
    std::string subtitle;
    std::string outputfile;
    std::string gradient;
    Imath::Vec3<float> background = Imath::Vec3<float>(0.0f, 0.0f, 0.0f);
    Imath::Vec3<float> color = Imath::Vec3<float>(1.0f, 1.0f, 1.0f);
    Imath::Vec2<int> size = Imath::Vec2<int>(1024, 1024);
    bool debug;
    int code = EXIT_SUCCESS;
};

static TitleTool tool;

// --title
static int
set_title(int argc, const char* argv[])
{
    OIIO_DASSERT(argc == 2);
    tool.title = argv[1];
    return 0;
}

static int
set_subtitle(int argc, const char* argv[])
{
    OIIO_DASSERT(argc == 2);
    tool.subtitle = argv[1];
    return 0;
}

static int
set_gradient(int argc, const char* argv[])
{
    OIIO_DASSERT(argc == 2);
    tool.gradient = argv[1];
    return 0;
}

// --outputfile
static int
set_outputfile(int argc, const char* argv[])
{
    OIIO_DASSERT(argc == 2);
    tool.outputfile = argv[1];
    return 0;
}

// --size
static int
set_size(int argc, const char* argv[])
{
    OIIO_DASSERT(argc == 2);
    std::istringstream iss(argv[1]);
    iss >> tool.size.x;
    iss.ignore(); // Ignore the comma
    iss >> tool.size.y;
    if (iss.fail()) {
        print_error("could not parse size from string: ", argv[1]);
        return 1;
    } else {
        return 0;
    }
}

// --help
static void
print_help(ArgParse& ap)
{
    ap.print_help();
}

// utils - filesystem
std::string font_path(const std::string& font)
{
    return Filesystem::parent_path(Sysutil::this_program_path()) + "/fonts/" + font;
}

// utils - drawing
Imath::Vec3<float> rgb_from_hsv(const Imath::Vec3<float>& hsv) {
    float hue = hsv.x;
    float saturation = hsv.y;
    float value = hsv.z;

    // Normalize the hue to be within [0, 360)
    if (hue >= 360.0f) hue = 0.0f;

    // Return black when value is 0
    if (value < std::numeric_limits<float>::epsilon()) {
        return Imath::Vec3<float>(0, 0, 0);
    }

    // Return gray when saturation is 0
    if (saturation < std::numeric_limits<float>::epsilon()) {
        return Imath::Vec3<float>(value, value, value);
    }

    int hi = static_cast<int>(std::floor(hue / 60.0f)) % 6;
    float f = (hue / 60.0f) - static_cast<float>(hi);
    float p = value * (1.0f - saturation);
    float q = value * (1.0f - f * saturation);
    float t = value * (1.0f - (1.0f - f) * saturation);

    float r = 0, g = 0, b = 0;
    switch (hi) {
        case 0: r = value, g = t, b = p; break;
        case 1: r = q, g = value, b = p; break;
        case 2: r = p, g = value, b = t; break;
        case 3: r = p, g = q, b = value; break;
        case 4: r = t, g = p, b = value; break;
        case 5: r = value, g = p, b = q; break;
    }

    return Imath::Vec3<float>(r, g, b);
}

void draw_gradient(ImageBuf &imagebuf, ROI roi,  Imath::Vec3<float> startcolor,  Imath::Vec3<float> endcolor) {
    for (int y = roi.ybegin; y < roi.yend; ++y) {
        float blend = static_cast<float>(y - roi.ybegin) / (roi.height() - 1);
        float r = (1 - blend) * startcolor[0] + blend * endcolor[0];
        float g = (1 - blend) * startcolor[1] + blend * endcolor[1];
        float b = (1 - blend) * startcolor[2] + blend * endcolor[2];
        for (int x = roi.xbegin; x < roi.xend; ++x) {
            imagebuf.setpixel(x, y, {r, g, b, 1.0f});
        }
    }
}

// main
int 
main( int argc, const char * argv[])
{
    // Helpful for debugging to make sure that any crashes dump a stack
    // trace.
    Sysutil::setup_crash_stacktrace("stdout");

    Filesystem::convert_native_arguments(argc, (const char**)argv);
    ArgParse ap;

    ap.intro("titletool -- a utility for creating title images\n");
    ap.usage("titletool [options] ...")
      .add_help(false)
      .exit_on_error(true);
    
    ap.separator("General flags:");
    ap.arg("--help", &tool.help)
      .help("Print help message");
    
    ap.arg("-v", &tool.verbose)
      .help("Verbose status messages");
    
    ap.arg("-d", &tool.debug)
      .help("Debug status messages");
    
    ap.separator("Input flags:");
    ap.arg("--title %s:TITLE")
      .help("Set title")
      .action(set_title);
    
    ap.arg("--subtitle %s:TITLE")
      .help("Set subtitle")
      .action(set_subtitle);
    
    ap.arg("--gradient %s:GRADIENT")
      .help("Set gradient")
      .action(set_gradient);
    
    ap.arg("--size %s:SIZE")
      .help("Set size (default: 1024, 1024)")
      .action(set_size);
    
    ap.separator("Output flags:");
    ap.arg("--outputfile %s:OUTPUTFILE")
      .help("Set output file")
      .action(set_outputfile);
    
    // clang-format on
    if (ap.parse_args(argc, (const char**)argv) < 0) {
        print_error(ap.geterror());
        print_help(ap);
        ap.abort();
        return EXIT_FAILURE;
    }
    if (ap["help"].get<int>()) {
        print_help(ap);
        ap.abort();
        return EXIT_SUCCESS;
    }
    
    if (!tool.outputfile.size()) {
        print_error("must have output file parameter");
        ap.briefusage();
        ap.abort();
        return EXIT_FAILURE;
    }
    if (argc <= 1) {
        ap.briefusage();
        print_error("\nFor detailed help: titletool --help\n");
        return EXIT_FAILURE;
    }

    // titletool program
    print_info("titletool -- a utility for creating title images");

    print_info("Writing title file: ", tool.outputfile);
    ImageSpec spec(tool.size.x, tool.size.y, 4, TypeDesc::FLOAT);
    ImageBuf imagebuf(spec);

    // title
    ROI roi(0, tool.size.x, 0, tool.size.y);
    int height = roi.height();
    int titlesize = height * 0.2;
    int subtitlesize = height * 0.1;
    int center = roi.ybegin + height / 2;
    int spacing = height * 0.08;

    // font
    std::string font = "Roboto.ttf";

    float hue = 49;
    
    // background
    bool found = false;
    if (tool.gradient.size() > 0)
    {
        std::map<std::string, float> hues;
        hues["red"] = 360.0f;
        hues["orange"] = 30.0f;
        hues["yellow"] = 60.0f;
        hues["green"] = 120.0f;
        hues["cyan"] = 180.0f;
        hues["azure"] = 210.0f;
        hues["blue"] = 240.0f;
        hues["violet"] = 270.0f;
        hues["magenta"] = 300.0f;
        hues["rose"] = 330.0f;
        print_info("tool.gradient: ", tool.gradient);
        std::map<std::string, float>::iterator it = hues.find(tool.gradient);
        if (it != hues.end()) {
            float hue = it->second;
            Imath::Vec3<float> a = rgb_from_hsv(Imath::Vec3<float>(hue, 0.9, 0.5));
            draw_gradient(
                    imagebuf,
                    roi,
                    rgb_from_hsv(Imath::Vec3<float>(hue, 1.0, 0.5)),
                    rgb_from_hsv(Imath::Vec3<float>(hue, 0.5, 0.8))
            );
            found = true;
        } else {
            print_warning("could not find hue for gradient: ", tool.gradient);
            std::string options;
            for (const std::pair<const std::string, float>& pair : hues) {
                if (options.size()) {
                    options += ", ";
                }
                options += pair.first;
            }
            print_warning("available options are: ", options);
        }
    }
    
    if (!found) {
        ImageBufAlgo::fill(
                imagebuf,
                { tool.background.x, tool.background.y, tool.background.z, 1.0f },
                roi
        );
    }
    
    // center
    int titley, subtitley;
    {
        ROI titleroi = ImageBufAlgo::text_size(tool.title, titlesize, font_path(font));
        ROI subtitleroi = ImageBufAlgo::text_size(tool.title, subtitlesize, font_path(font));
        int textheight = titleroi.height() + spacing + subtitleroi.height();
        titley = center - (textheight / 2);
        subtitley = titley + titleroi.height() + spacing;
    }
    
    // title
    {
        std::ostringstream oss;
        oss << "size: "
            << tool.size.x
            << ", "
            << tool.size.y
            << " ";

        ImageBufAlgo::render_text(
            imagebuf,
            roi.xbegin + roi.width() / 2, // Center horizontally
            titley,
            tool.title,
            titlesize,
            font_path(font),
            { tool.color.x, tool.color.y, tool.color.z, 1.0f },
            ImageBufAlgo::TextAlignX::Center,
            ImageBufAlgo::TextAlignY::Top
        );
    }
    
    // subtitle
    {
        std::ostringstream oss;
        oss << "size: "
            << tool.size.x
            << ", "
            << tool.size.y
            << " ";
            
        ImageBufAlgo::render_text(
            imagebuf,
            roi.xbegin + roi.width() / 2, // Center horizontally
            subtitley,
            tool.subtitle,
            subtitlesize,
            font_path(font),
            { tool.color.x, tool.color.y, tool.color.z, 1.0f },
            ImageBufAlgo::TextAlignX::Center,
            ImageBufAlgo::TextAlignY::Top
        );
    }
    
    if (!imagebuf.write(tool.outputfile)) {
        print_error("could not write output file", imagebuf.geterror());
    }
    return 0;
}
