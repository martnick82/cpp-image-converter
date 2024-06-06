#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}
PACKED_STRUCT_BEGIN BitmapFileHeader{
    char sign1;
    char sign2;
    uint32_t size;
    uint32_t padding;
    uint32_t indent;
}
PACKED_STRUCT_END

bool CheckFileHeader(const BitmapFileHeader& header)
{
    if (header.sign1 != 'B' || header.sign2 != 'M' || header.padding != BMP_HEAD_PADDING || header.indent != BMP_INDENT) {
        return false;
    }
    return true;
}

PACKED_STRUCT_BEGIN BitmapInfoHeader{
    uint32_t info_head_indent;
    int32_t width;
    int32_t height;
    uint16_t plane_count;
    uint16_t color_depth;
    uint32_t compress_depth;
    uint32_t size;
    int32_t h_res;
    int32_t v_res;
    int32_t num_using_colors;
    int32_t num_significant_colors;
}
PACKED_STRUCT_END

bool CheckInfoHeader(const BitmapInfoHeader& header)
{
    if (header.info_head_indent != BMP_INFO_HEAD_INDENT
        || header.plane_count != BMP_PLANE_COUNT
        || header.color_depth != BMP_COLOR_DEPTH
        || header.compress_depth != BMP_COMPRESS_DEPTH
        || header.h_res != BMP_H_RES
        || header.v_res != BMP_V_RES
        || header.num_using_colors != BMP_NUM_USING_COLORS
        || header.num_significant_colors != BMP_NUM_SIGNIFICANT_COLORS)  {
        return false;
    }
    return true;
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image)
{
    ofstream out(file, ios::binary);
    if (!out)
    {
        return false;
    }
    int32_t w = image.GetWidth();
    int32_t h = image.GetHeight();
    uint32_t step = GetBMPStride(w);

    BitmapFileHeader file_header = {
                            'B', 'M',
                            sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + step * h,
                            BMP_HEAD_PADDING,
                            BMP_INDENT                            
    };

    out.write((char*) & file_header, sizeof(file_header));

    BitmapInfoHeader info_header = {
                        BMP_INFO_HEAD_INDENT,
                        w,
                        h,
                        BMP_PLANE_COUNT,
                        BMP_COLOR_DEPTH,
                        BMP_COMPRESS_DEPTH,
                        h * step,
                        BMP_H_RES,
                        BMP_V_RES,
                        BMP_NUM_USING_COLORS,
                        BMP_NUM_SIGNIFICANT_COLORS
    };

    out.write((char*)&info_header, sizeof(info_header));

    std::vector<char> buff(step);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);

        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = (static_cast<char>((line + x)->b));
            buff[x * 3 + 1] = (static_cast<char>((line + x)->g));
            buff[x * 3 + 2] = (static_cast<char>((line + x)->r));
            //out <<  << static_cast<char>((line + x)->g) << static_cast<char>((line + x)->r);
        }
        out.write(buff.data(), step);
    }
    return true;
}

// напишите эту функцию
Image LoadBMP(const Path& file)
{
    // открываем поток с флагом ios::binary
    // поскольку будем читать данные в двоичном формате
    ifstream ifs(file, ios::binary);

    BitmapFileHeader file_header;
    ifs.read((char*)&file_header, sizeof(file_header));
    if (!CheckFileHeader(file_header))
    {
        return {};
    }

    BitmapInfoHeader info_header;
    ifs.read((char*)&info_header, sizeof(info_header));
    if (!CheckInfoHeader(info_header))
    {
        return {};
    }
    int32_t w = info_header.width;
    int32_t h = info_header.height;

    uint16_t planes;

    int step = GetBMPStride(w);

    Image result(w, h, Color::Black());
    std::vector<char> buff(w * 3);

    for (int y = h - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), w*3);

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
        ifs.read(buff.data(), step - w*3);
    }
    ifs.close();
    return result;
}

}  // namespace img_lib