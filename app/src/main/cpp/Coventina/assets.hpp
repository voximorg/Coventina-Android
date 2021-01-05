#pragma once

#include <map>

#include "data.h"

struct Buffer {
    const unsigned char* data;
    uint32_t size;
};

typedef std::map<std::string, Buffer> AssetsMap;
AssetsMap assets = {
        {"bricks.png",
                {bricks_png, bricks_png_len}
        },
        {
            "Cube.holden",
                {Cube_holden, Cube_holden_len}
        },
        {
            "CuttelFish.holden",
                {CuttelFish_holden, CuttelFish_holden_len}
        },
        {
            "floorgrid.png",
                {floorgrid_png, floorgrid_png_len}
        },
        {
            "ggj-splash.png",
                {ggj_splash_png, ggj_splash_png_len}
        },
        {
            "gold.png",
                {gold_png, gold_png_len}
        },
        {
                "grass.png",
                {grass_png, grass_png_len}
        },
        {
                "guitar.holden",
                {guitar_holden, guitar_holden_len}
        },
        {
                "manScaled2.holden",
                {manScaled2_holden, manScaled2_holden_len}
        },
        {
                "map.in",
                {map_in, map_in_len}
        },
        {
                "MapTexture.png",
                {MapTexture_png, MapTexture_png_len}
        },
        {
                "maze.grid",
                {maze_grid, maze_grid_len}
        },
        {
                "Ring.holden",
                {Ring_holden, Ring_holden_len}
        },
        {
                "ui/thumbstick.png",
                {ui_thumbstick_png, ui_thumbstick_png_len}
        }
};


#include <streambuf>
#include <istream>
#include <android/log.h>

class membuf : public std::basic_streambuf<char> {
public:
    membuf(const uint8_t *p, size_t l) {
        setg((char*)p, (char*)p, (char*)p + l);
    }
};

class memstream : public std::istream {
public:
    memstream(const uint8_t *p, size_t l) :
            std::istream(&_buffer),
            _buffer(p, l) {
        rdbuf(&_buffer);
    }

    memstream(const Buffer& buf):
        memstream(buf.data, buf.size)
    {

    }

    memstream(const std::string& path):
        memstream(check_(path)->second)
    {}

private:
    membuf _buffer;

    AssetsMap::iterator check_(const std::string& path) {
        auto i = assets.find(path);
        if (i == assets.end()) {
            __android_log_print(ANDROID_LOG_ERROR, "Assets", "Can't find asset at %s", path.c_str());
            exit(1);
        }

        return i;
    }
};

