#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    //ЗАМЕЧАНИЕ: Лучше количество цветов и выравнивание стоит вынести в именованные переменные или константы для читаемости
    //Пояснение: Согласен, так лучше. Ввёл константы в ImgLib и bmp_image
    return BMP_STRIDE_ALIGNMENT * ((w * BYTES_PER_COLOR + BMP_STRIDE_ALIGNMENT - 1) / BMP_STRIDE_ALIGNMENT);
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
    if (!out.good())
    {
        return false;
    }

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
    if (!out.good())
    {
        return false;
    }

    std::vector<char> buff(step);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);

        for (int x = 0; x < w; ++x) {
            buff[x * BYTES_PER_COLOR + 0] = (static_cast<char>((line + x)->b));
            buff[x * BYTES_PER_COLOR + 1] = (static_cast<char>((line + x)->g));
            buff[x * BYTES_PER_COLOR + 2] = (static_cast<char>((line + x)->r));
        }
        out.write(buff.data(), step);
        if (!out.good())
        {
            return false;
        }
    }
    return true;
    // ЗАМЕЧАНИЕ: И с чего это так ? У вас вообще запись может провалиться, в таком случае - вам надо проверить 
    // состояние потока и вернуть false
    // Пояснение: Тут и сказать нечего. Ввёл проверки потоков после операций чтения/записи
}

Image LoadBMP(const Path& file)
{
    ifstream ifs(file, ios::binary);

    BitmapFileHeader file_header;
    ifs.read((char*)&file_header, sizeof(file_header));
    if (!ifs.good())
    {
        return {};
    }
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

    int step = GetBMPStride(w);

    Image result(w, h, Color::Black());
    std::vector<char> buff(w * BYTES_PER_COLOR);

    for (int y = h - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        // ЗАМЕЧАНИЕ: Здесь и ниже не хватает проверки состояния потока - чтение может проваливаться
        //  и это через методы стоит проверять.
        // Пояснение: сделал проверки после чтения
        ifs.read(buff.data(), w * BYTES_PER_COLOR);
        if (!ifs.good())
        {
            return {};
        }
        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * BYTES_PER_COLOR + 0]);
            line[x].g = static_cast<byte>(buff[x * BYTES_PER_COLOR + 1]);
            line[x].r = static_cast<byte>(buff[x * BYTES_PER_COLOR + 2]);
        }
        ifs.read(buff.data(), step - w * BYTES_PER_COLOR);
        if (!ifs.good())
        {
            return {};
        }
    }
    ifs.close();
    return result;
}

}  // namespace img_lib