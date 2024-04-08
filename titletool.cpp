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
print_info(std::string param, const T& value)
{
    std::cout << "info: " << param << value << std::endl;
}

template <typename T>
static void
print_warning(std::string param, const T& value)
{
    std::cout << "warning: " << param << value << std::endl;
}

template <typename T>
static void
print_error(std::string param, const T& value)
{
    std::cerr << "error: " << param << value << std::endl;
}

// title tool
struct TitleTool
{
    bool help = false;
    bool verbose = false;
    std::string title;
    std::string subtitle;
    std::string outputfile;
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
    
    ap.arg("--size %s:SIZE")
      .help("Set size (default: 1024, 1024)")
      .action(set_size);
    
    ap.separator("Output flags:");
    ap.arg("--outputfile %s:OUTPUTFILE")
      .help("Set output file")
      .action(set_outputfile);
    
    // clang-format on
    if (ap.parse_args(argc, (const char**)argv) < 0) {
        std::cerr << "error: " << ap.geterror() << std::endl;
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
        std::cerr << "error: must have output file parameter\n";
        ap.briefusage();
        ap.abort();
        return EXIT_FAILURE;
    }
    if (argc <= 1) {
        ap.briefusage();
        std::cout << "\nFor detailed help: titletool --help\n";
        return EXIT_FAILURE;
    }

    // titletool program
    std::cout << "titletool -- a utility for creating title images" << std::endl;

    print_info("Writing title file: ", tool.outputfile);
    ImageSpec spec(tool.size.x, tool.size.y, 4, TypeDesc::FLOAT);
    ImageBuf imagebuf(spec);

    // title
    ROI roi(0, tool.size.x, 0, tool.size.y);
    int height = roi.height();
    int titlesize = height * 0.08;
    int subtitlesize = height * 0.04;
    int center = roi.ybegin + height / 2;
    int spacing = height * 0.02;

    // background
    ImageBufAlgo::fill(
            imagebuf,
            { tool.background.x, tool.background.y, tool.background.z, 1.0f },
            roi
    );
    
    // center
    int titley, subtitley;
    {
        ROI titleroi = ImageBufAlgo::text_size(tool.title, titlesize, "../Roboto.ttf");
        ROI subtitleroi = ImageBufAlgo::text_size(tool.title, titlesize, "../Roboto.ttf");
        int textheight = titleroi.height() + spacing + subtitleroi.height();
        int y = center - (textheight / 2);

        
        
        titley = y;
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
            "../Roboto.ttf",
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
            "../Roboto.ttf",
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
