#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>
#include <bmp_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>

using namespace std;

enum Format
{ UNKNOWN, PPM, JPEG, BMP };

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};


class JpegFormatInterface : public ImageFormatInterface
{
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override;
    img_lib::Image LoadImage(const img_lib::Path& file) const override;
};

img_lib::Image JpegFormatInterface::LoadImage(const img_lib::Path& file) const
{
    if (GetFormatByExtension(file) == Format::JPEG)
    {
        return img_lib::LoadJPEG(file);
    }
    return {};
}

bool JpegFormatInterface::SaveImage(const img_lib::Path& file, const img_lib::Image& image) const
{
    if (GetFormatByExtension(file) == Format::JPEG)
    {
        return img_lib::SaveJPEG(file, image);
    }
    return false;
}

class PpmFormatInterface : public ImageFormatInterface
{
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override;
    img_lib::Image LoadImage(const img_lib::Path& file) const override;
};

img_lib::Image PpmFormatInterface::LoadImage(const img_lib::Path& file) const
{
    if (GetFormatByExtension(file) == Format::PPM)
    {
        return img_lib::LoadPPM(file);
    }
    return {};
}

bool PpmFormatInterface::SaveImage(const img_lib::Path& file, const img_lib::Image& image) const
{
    if (GetFormatByExtension(file) == Format::PPM)
    {
        img_lib::SavePPM(file, image);
        return true;
    }
    return false;
}

class BmpFormatInterface : public ImageFormatInterface
{
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override;
    img_lib::Image LoadImage(const img_lib::Path& file) const override;
};

img_lib::Image BmpFormatInterface::LoadImage(const img_lib::Path& file) const
{
    if (GetFormatByExtension(file) == Format::BMP)
    {
        return img_lib::LoadBMP(file);
    }
    return {};
}

bool BmpFormatInterface::SaveImage(const img_lib::Path& file, const img_lib::Image& image) const
{
    if (GetFormatByExtension(file) == Format::BMP)
    {
        return img_lib::SaveBMP(file, image);
    }
    return false;
}

ImageFormatInterface* GetFormatInterface(const img_lib::Path& path)
{
    Format format = GetFormatByExtension(path);
    switch (format)
    {
    case Format::PPM:
    {
        return new PpmFormatInterface();
    }
    case Format::JPEG:
    {
        return new JpegFormatInterface();
    }
            
    case Format::BMP:
    {
        return new BmpFormatInterface();
    }        
            
    case Format::UNKNOWN:{}    
    default:
    {
        return nullptr;
    }
    }

}

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    ImageFormatInterface* in_interface = GetFormatInterface(in_path);
    ImageFormatInterface* out_interface = GetFormatInterface(out_path);

    if (!in_interface)
    {
        cerr << "Unknown format of the input file"sv << endl;
        return 2;
    }

    if (!out_interface)
    {
        cerr << "Unknown format of the output file"sv << endl;
        return 3;
    }

    img_lib::Image image = in_interface->LoadImage(in_path);

    if (!image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!out_interface->SaveImage(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
}